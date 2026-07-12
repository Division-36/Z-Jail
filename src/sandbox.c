#include "z_jail.h"
#include <sys/resource.h>
#include <sys/mount.h>
#include <linux/securebits.h>
#include <linux/limits.h>

static int pivot_into(const char *new_root)
{
    char put_old[PATH_MAX];
    snprintf(put_old, sizeof(put_old), "%s/.pivot_old", new_root);
    if (mkdir(put_old,0755)<0 && errno!=EEXIST)
        { axiom_log(LOG_ERROR,"pivot: mkdir %s: %s\n",put_old,strerror(errno)); return -1; }
    if (mount(NULL,"/",NULL,MS_REC|MS_PRIVATE,NULL)<0)
        { axiom_log(LOG_ERROR,"pivot: make-rprivate: %s\n",strerror(errno)); return -1; }
    if (mount(new_root,new_root,NULL,MS_BIND|MS_REC,NULL)<0)
        { axiom_log(LOG_ERROR,"pivot: bind mount: %s\n",strerror(errno)); return -1; }
    if (syscall(SYS_pivot_root,new_root,put_old)<0)
        { axiom_log(LOG_ERROR,"pivot: pivot_root: %s\n",strerror(errno)); return -1; }
    if (chdir("/")<0)
        { axiom_log(LOG_ERROR,"pivot: chdir /: %s\n",strerror(errno)); return -1; }
    if (umount2("/.pivot_old",MNT_DETACH)<0)
        axiom_log(LOG_WARN,"pivot: umount: %s\n",strerror(errno));
    rmdir("/.pivot_old");
    return 0;
}

static int drop_caps(void)
{
    struct __user_cap_header_struct cap_hdr;
    struct __user_cap_data_struct cap_data[2];
    if (setgid(65534) < 0 || setuid(65534) < 0)
        { axiom_log(LOG_ERROR,"caps: setuid: %s\n",strerror(errno)); return -1; }
    memset(&cap_hdr,0,sizeof(cap_hdr));
    memset(cap_data,0,sizeof(cap_data));
    cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
    cap_hdr.pid = 0;
    if (syscall(SYS_capset, &cap_hdr,cap_data)<0)
        { axiom_log(LOG_ERROR,"caps: capset: %s\n",strerror(errno)); return -1; }
    prctl(PR_SET_SECUREBITS,
        SECBIT_KEEP_CAPS_LOCKED|SECBIT_NO_SETUID_FIXUP|SECBIT_NO_SETUID_FIXUP_LOCKED
        |SECBIT_NOROOT|SECBIT_NOROOT_LOCKED, 0,0,0);
    return 0;
}

int child_run(void *arg)
{
    sandbox_config *cfg = (sandbox_config *)arg;
    struct rlimit rl; int i;
    rl.rlim_cur=16;rl.rlim_max=16;
    if(setrlimit(RLIMIT_NOFILE,&rl)<0){_exit(AXIOM_CHILD_ERR_SETUP);}
    rl.rlim_cur=cfg->as_limit;rl.rlim_max=cfg->as_limit;
    if(setrlimit(RLIMIT_AS,&rl)<0){_exit(AXIOM_CHILD_ERR_SETUP);}
    rl.rlim_cur=(rlim_t)cfg->timeout_sec;rl.rlim_max=(rlim_t)cfg->timeout_sec;
    if(setrlimit(RLIMIT_CPU,&rl)<0){_exit(AXIOM_CHILD_ERR_SETUP);}
    rl.rlim_cur=1;rl.rlim_max=1;
    if(setrlimit(RLIMIT_NPROC,&rl)<0){_exit(AXIOM_CHILD_ERR_SETUP);}
    for(i=3;i<1024;i++) if(i!=cfg->report_fd) close(i);
    prctl(PR_SET_DUMPABLE,0,0,0,0);
    if(pivot_into(cfg->root_path)<0){_exit(AXIOM_CHILD_ERR_PERM);}
    prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0);
    if(drop_caps()<0){_exit(AXIOM_CHILD_ERR_CAP);}
    if(cfg->seccomp_enforce && apply_whitelist()<0){_exit(AXIOM_CHILD_ERR_SECCOMP);}
    {unsigned char ready=1;AXIOM_IGNORE_RESULT(write(cfg->report_fd,&ready,1));}
    execve(cfg->exec_path,cfg->exec_argv,cfg->exec_envp);
    _exit(AXIOM_CHILD_ERR_EXEC);
}
