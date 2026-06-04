#include "z_jail.h"
#include <sys/wait.h>
extern char **environ;
#define CHILD_STACK_SIZE (1024*1024)

static int parse_args(int argc,char**argv,sandbox_config*cfg,char**self_hash_hex,int *quiet)
{
    int i,cmd_start=-1; *self_hash_hex=NULL; *quiet=0;
    cfg->root_path=".";cfg->seccomp_enforce=0;cfg->timeout_sec=30;cfg->as_limit=256ULL*1024*1024;
    for(i=1;i<argc;i++){
        if(strcmp(argv[i],"--")==0){cmd_start=i+1;break;}
        if(strncmp(argv[i],"--root=",7)==0)cfg->root_path=argv[i]+7;
        else if(strcmp(argv[i],"--seccomp-enforce")==0)cfg->seccomp_enforce=1;
        else if(strncmp(argv[i],"--self-hash=",12)==0)*self_hash_hex=argv[i]+12;
        else if(strcmp(argv[i],"--quiet")==0)*quiet=1;
        else if(strcmp(argv[i],"--verbose")==0)axiom_log_level=LOG_DEBUG;
        else if(strcmp(argv[i],"--help")==0){printf("Usage: z_jail --root=<dir> [--seccomp-enforce] [--self-hash=<hex>] [--quiet] [--verbose] -- <program> [args...]\n");exit(0);}
        else if(strcmp(argv[i],"--version")==0){printf("%s\n",ZJAIL_BUILD_ID);exit(0);}
        else if(argv[i][0]=='-'){fprintf(stderr,"unknown: %s\n",argv[i]);return -1;}
        else{cmd_start=i;break;}
    }
    if(cmd_start<0||cmd_start>=argc){fprintf(stderr,"Usage: z_jail --root=<dir> -- <program> [args...]\n");return -1;}
    cfg->exec_path=argv[cmd_start];cfg->exec_argv=argv+cmd_start;cfg->exec_envp=environ;
    return 0;
}
static const char*verdict_str(axiom_verdict v){
    switch(v){
        case AXIOM_VERDICT_DETERMINISTIC:return"DETERMINISTIC";
        case AXIOM_VERDICT_REJECT:return"REJECT";
        case AXIOM_VERDICT_UNCERTAIN:return"UNCERTAIN";
        default:return"UNKNOWN";
    }
}
static int do_self_hash(const char *hex){
    uint8_t expected[32],actual[32];
    if(!hex)return 0;
    if(axiom_hex_decode(hex,expected,32)<0){fprintf(stderr,"bad hex\n");return -1;}
    if(axiom_blake2b_file("/proc/self/exe",actual)<0){fprintf(stderr,"no hash\n");return -1;}
    if(memcmp(expected,actual,32)!=0){fprintf(stderr,"SELF-HASH MISMATCH\n");return -2;}
    axiom_log(LOG_INFO,"self-hash: OK\n");
    return 0;
}
int main(int argc,char**argv){
    sandbox_config cfg;char*self_hash_hex=NULL;axiom_audit*audit=NULL;
    int pipe_fds[2];pid_t child_pid;int status;long long start_ns,end_ns;
    void*child_stack;axiom_verdict verdict;char audit_json[8192];
    uint8_t file_hash[32];char file_hash_hex[65];char audit_path[4096];
    const char*exe_name;int quiet;
    if(parse_args(argc,argv,&cfg,&self_hash_hex,&quiet)<0)return 1;
    {int r=do_self_hash(self_hash_hex);if(r<0){return r==-2?3:2;}}
    exe_name=strrchr(cfg.exec_path,'/');exe_name=exe_name?exe_name+1:cfg.exec_path;
    snprintf(audit_path,sizeof(audit_path),"build/audits/%s.audit.json",exe_name);
    audit=axiom_audit_open(audit_path);
    if(pipe2(pipe_fds,O_CLOEXEC)<0){perror("pipe2");return 1;}
    cfg.report_fd=pipe_fds[1];
    child_stack=malloc(CHILD_STACK_SIZE);if(!child_stack){perror("malloc");return 1;}
    start_ns=axiom_epoch_ns();
    child_pid=clone(child_run,(char*)child_stack+CHILD_STACK_SIZE,
        CLONE_NEWNS|CLONE_NEWPID|CLONE_NEWNET|CLONE_NEWIPC|CLONE_NEWUTS|SIGCHLD,&cfg);
    if(child_pid<0){perror("clone");free(child_stack);return 1;}
    close(pipe_fds[1]);cfg.report_fd=-1;
    {char ready=0;AXIOM_IGNORE_RESULT(read(pipe_fds[0],&ready,1));}close(pipe_fds[0]);
    waitpid(child_pid,&status,0);free(child_stack);end_ns=axiom_epoch_ns();
    verdict=(WIFEXITED(status)&&WEXITSTATUS(status)==0)?AXIOM_VERDICT_DETERMINISTIC:AXIOM_VERDICT_REJECT;
    if(axiom_blake2b_file(cfg.exec_path,file_hash)==0)axiom_hex_encode(file_hash,32,file_hash_hex);
    else snprintf(file_hash_hex,sizeof(file_hash_hex),"unavailable");
    snprintf(audit_json,sizeof(audit_json),
        "{\"schema\":\"z-jail.audit/v1\",\"build_id\":\""ZJAIL_BUILD_ID"\""
        ",\"timestamp\":%lld,\"duration_ns\":%lld"
        ",\"executable\":\"%s\",\"verdict\":\"%s\",\"exit_code\":%d"
        ",\"sandbox\":{\"seccomp_filter\":\"whitelist-v1\""
        ",\"seccomp_whitelist_size\":%d,\"seccomp_arg_rules_size\":%d"
        ",\"namespaces\":[\"mount\",\"pid\",\"net\",\"ipc\",\"uts\"]"
        ",\"pivot_root\":\"%s\",\"no_new_privs\":true,\"capabilities_dropped\":true"
        "},\"content_fingerprint\":\"%s\"}"
        ,(long long)time(NULL),end_ns-start_ns,cfg.exec_path,verdict_str(verdict)
        ,WIFEXITED(status)?WEXITSTATUS(status):-WTERMSIG(status)
        ,axiom_whitelist_size,axiom_arg_rules_size,cfg.root_path,file_hash_hex);
    if(!quiet&&audit)axiom_audit_write(audit,"sessions",audit_json);
    axiom_audit_close(audit);
    return WIFEXITED(status)?WEXITSTATUS(status):1;
}
