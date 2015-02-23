#include <fcntl.h>
#include <sys/stat.h>
#include "ovsCommDef.h"
#include "ovsLogMgr.h"
#include "ovsMemMgr.h"
#include "ovsThreadMgr.h"
#include "ovsRsrc.h"
#include "rsrcMgrApiServer.h"
#include "ovsRsrcMgr.h"

void usage( const char* program ) {
	cout << "usage: " << program << " [options]" << endl;
	cout << endl;
	cout << "Option list:" << endl;
	cout << "-h, -?                        print this help" << endl;
	cout << "-d			 	      runs OVS Resource Manager in daemon mode" << endl;
	cout << "-o output file              write logging output into file" << endl;
	cout << "-l log level                  enable given list of loglevels" << endl;
	cout << "-i pid_file               	set process identifier file name" << endl;
	cout << "-v					display program version information" << endl;
	cout << endl;
	cout << "for log levels, choose from:" << endl;
	ovsLogMgr::usage(cout);
	cout << endl;
}

void version(char* progname)
{
	cout << progname << " version " <<  OVS_RSRC_MGR_VERSION << endl;
	cout << "Copyright 2014-2014, CoAdna Photonics Inc." << endl;
}

/* Daemonize myself. */
int
daemonize(int nochdir, int noclose)
{
  pid_t pid;

  /* 
    * Fork a child process.
    */
  pid = fork();

  if (pid < 0)
	{
	  perror ("fork");
	  return(-1);
	}

  /* 
    * this is parent process. 
    */
  if (pid != 0)
	exit(0);

  /* 
    * this is child process. 
    */
  pid = setsid();

  if (pid < -1)
	{
	  perror ("setsid");
	  return(-1);
	}

  /* 
    * Change direovsory to root. 
    */
  if (! nochdir)
	chdir ("/");

  /* 
    * File descriptor close. 
    */
  if (! noclose)
  {
    int fd;
  
    fd = open ("/dev/null", O_RDWR, 0);
    if (fd != -1)
    {
      dup2 (fd, STDIN_FILENO);
      dup2 (fd, STDOUT_FILENO);
      dup2 (fd, STDERR_FILENO);
      if (fd > 2)
        close (fd);
    }
  }

  umask (0027);

  return 0;
}

int main( int argc, char** argv ) 
{
    const char* loglevel = "info";
    const char* logfile = OVS_RSRC_MGR_LOGFILE;
    const char* pidfile = RSRC_MGR_PID_FILE;
    char *progname, *p;
    int daemon = 0;

    /* 
      * Set umask before anything for security 
      */
    umask(0027);

    /* 
      * get program name, exclude path from the name
      */
    progname = ((p = strrchr(argv[0], '/')) ? ++p : argv[0]);
	
    for (;;) 
    {
    	int option = getopt(argc, argv, "?hdol:p:i:v" );
    	if ( option == -1 ) 
          break;
		
    	switch(option) 
	{
    	case 'l':
    		loglevel = optarg;
    		break;
    	case 'o':
    		logfile = ((optarg != NULL) ? optarg : logfile);
    		break;
    	case 'd':
    		daemon = 1;
    		break;
       case 'i':
             pidfile = optarg;
             break;
	case 'v':
             version(progname);
             exit(0);
             break;
    	default:
    		usage(progname);
    		exit(0);
    	}
    }
    if (daemon)
    	daemonize(0, 0);

    /*
      * write pid to file.
      */
    ostream *pidstream = new ofstream(pidfile);
    if ( pidstream->good() ) 
    {
    	*pidstream << getpid() << endl;
    }
    delete(pidstream);

    /*
      * start log manager
      */
    ovsLogMgr* logMgr = ovsLogMgr::instance();
    if (logMgr == NULL)
    {
        cout << "[RsrcMgr] unable to start log manager. Aborted." << endl;
	exit(-1);
    }
    logMgr->setLogLevel(loglevel);
    if (logfile != NULL)
    {
       logMgr->setLogOutput(OVS_LOG_OUTPUT_FILE);
       logMgr->setFileLogInfo(logfile);
    }
	
    OVS_LOG_INFO(NULL, OVS_LOG_TYPE_RSRCMGR, 
		        "LogMgr started.");
   
    /*
      * start memory manager
      */
    ovsMemMgr* memMgr = ovsMemMgr::instance();
    if (memMgr == NULL)
    {
        cout << "[RsrcMgr] unable to start memory manager. Aborted." << endl;
	exit(-1);
    }

    /*
      * start thread manager
      */
    ovsThreadMgr* thrMgr = ovsThreadMgr::instance();
    if (thrMgr == NULL)
    {
        cout << "[RsrcMgr] unable to start thread manager. Aborted." << endl;
	exit(-1);
    }

    /* 
      * start OVS Resource Manager
      */
    ovsRsrcMgr* rmgr = ovsRsrcMgr::instance();
    if (rmgr == NULL)
    {
        cout << "[RsrcMgr] unable to start Resource Manager. Aborted." << endl;
	exit(-1);
    }

    /* 
      * start Resource Manager API Server
      */
    rsrcMgrApiServer* apiSvr = rsrcMgrApiServer::instance();
    if (apiSvr == NULL)
    {
        cout << "[RsrcMgr] unable to start resource manager API server. Aborted." << endl;
	exit(-1);
    }

    /* 
      * enters thread loop and run until the end of the world.
      */
    ovsThread* thr;
    while ((thr = thrMgr->fetch()) != NULL)
        thr->run();
        
    /* 
      * should never be reached. If so then god will appear.:-)
      */
    delete(apiSvr);
    delete(rmgr);
    delete(thrMgr);
    delete(memMgr);
    delete(logMgr);

    unlink(RSRC_MGR_PID_FILE);

    return(0);
}



