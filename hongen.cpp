#include "stdafx.h"

#define MAX_CMDLINE (3 * 1024)
#define HOGAN_CMDLINE_FILE_ENV_NAME L"HOGEN_CMDLINE_FILE"
#define CHK(_desc_, _x_) if (!(_x_)) { printf("%s: 0x%x: %ws", _desc_, GetLastError(), strerror()); exit(1); }

static void create_cmdline(int argc, wchar_t** argv, wchar_t* dst, int dst_len);
static void start_child(wchar_t* cmdline);
static void print_usage(wchar_t* progname);
static wchar_t* strerror();

/**
 * Main
 */
int _tmain(int argc, wchar_t* argv[])
{
  wchar_t cmdline[sizeof(wchar_t) * (MAX_CMDLINE + 1)];

  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }
  else {
    /* Extract command line from argv[1] and forward */
    create_cmdline(argc, argv, cmdline, MAX_CMDLINE);
  }

  /* Execute once */
  printf("starting: %ws\n", cmdline);
  start_child(cmdline);

  return 0;
}

/**
 * Starts the executable `app_name` with command line `cmdline` (which should include the executable as well).
 */
void start_child(wchar_t* non_expended_cmdline) {
  wchar_t cmdline[sizeof(wchar_t) * (MAX_CMDLINE + 1)];
  JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_limit_info;
  PROCESS_INFORMATION proc_info;
  STARTUPINFO startup;
  HANDLE job_object;

  /* Expand environment vars (%XX%) in command line */
  CHK("ExpandEnvironmentStrings", ExpandEnvironmentStrings(non_expended_cmdline, cmdline, MAX_CMDLINE));

  /* Create job object for the child */
  job_object = CreateJobObject(NULL, NULL);
  CHK("CreateJobObject", job_object != NULL);

  /* Setup the job such that if the job handle is closed, all children are killed */
  memset(&job_limit_info, 0, sizeof(job_limit_info));
  job_limit_info.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
  CHK("SetInformationJobObject", SetInformationJobObject(job_object, JobObjectExtendedLimitInformation, &job_limit_info, sizeof(job_limit_info)));

  /* Start the child suspended, so we can associate it with the job object */
  memset(&startup, 0, sizeof(STARTUPINFO));
  startup.cb = sizeof(startup);
  CHK("CreateProcess", CreateProcess(
    NULL, cmdline, NULL, NULL, FALSE, 
    CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED | CREATE_BREAKAWAY_FROM_JOB,
    NULL, NULL, &startup, &proc_info));

  /* Associate process to job object */
  CHK("AssignProcessToJobObject", AssignProcessToJobObject(job_object, proc_info.hProcess));

  /* Resume thread */
  CHK("ResumeThread", ResumeThread(proc_info.hThread) != -1);

  /* Close thread handle (we don't care about it */
  CHK("CloseHandle(thread)", CloseHandle(proc_info.hThread));

  /* Wait for child to exit */
  WaitForSingleObject(proc_info.hProcess, INFINITE);

  /* Close the job object (will kill all decendents because of JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE) */
  CHK("CloseHandle(job)", CloseHandle(job_object));
  CHK("CloseHandle(process)", CloseHandle(proc_info.hProcess));
}

/**
 * Creates the command line of the program to start by concatenating all args
 * and adding "" if there's a whitespace.
 */
void create_cmdline(int argc, wchar_t** argv, wchar_t* dst, int dst_len) {
  int i;

  /* Clear `dst` */
  if (dst_len <= 0) return;
  dst[0] = '\0';

  for (i = 1; i < argc; ++i) {
    int hasws = wcschr(argv[i], L' ') != NULL;
    if (hasws) wcscat_s(dst, dst_len, L"\"");
    wcscat_s(dst, dst_len, argv[i]);
    if (hasws) wcscat_s(dst, dst_len, L"\"");
    wcscat_s(dst, dst_len, L" ");
  }
}

/**
 * Returns last windows error string
 */
wchar_t* strerror() {
  static wchar_t message[1024];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, message, sizeof(message) / sizeof(wchar_t), NULL);
  return message;
}

/**
 * Prints the help message
 */
void print_usage(wchar_t* progname) {
  wprintf(L"Usage: ");
  wprintf(L"%ws <command line with any arguments you want>\n", progname);
}
