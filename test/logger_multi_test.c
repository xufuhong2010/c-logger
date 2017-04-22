#include "logger.h"
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
 #include <io.h>
#else
 #include <fcntl.h>
 #include <unistd.h>
#endif /* defined(_WIN32) || defined(_WIN64) */
#include "nanounit.h"

#if defined(_WIN32) || defined(_WIN64)
#define dup _dup
#define dup2 _dup2
#define fileno _fileno
#endif /* defined(_WIN32) || defined(_WIN64) */

static const char* kConsoleOutputFileName = "console.log";
static const char* kFileOutputFileName = "file.log";

static void setup(void)
{
    remove(kConsoleOutputFileName);
    remove(kFileOutputFileName);
}

static void cleanup(void)
{
    remove(kConsoleOutputFileName);
    remove(kFileOutputFileName);
}

static int checkOnlyOneLineWritten(const char* filename, const char* message);

static int test_multiLogger(void)
{
    const char message[] = "message";
    int stdoutfd;
    FILE* redirect;
    int result;

    /* setup: redirect stdout to a file */
    stdoutfd = dup(1);
    if ((redirect = fopen(kConsoleOutputFileName, "w")) == NULL) {
        nu_fail();
    }
    dup2(fileno(redirect), 1);

    /* and: auto flush on */
    logger_autoFlush(10);

    /* when: initialize console logger */
    result = logger_initConsoleLogger(stdout);

    /* then: ok */
    nu_assert_eq_int(1, result);

    /* when: initialize file logger */
    result = logger_initFileLogger(kFileOutputFileName, 0, 0);

    /* then: ok */
    nu_assert_eq_int(1, result);

    /* when: output to stdout */
    LOG_TRACE(message);
    LOG_DEBUG(message);
    LOG_INFO(message);

    /* then: write only one line */
    checkOnlyOneLineWritten(kConsoleOutputFileName, message);
    checkOnlyOneLineWritten(kFileOutputFileName, message);

    /* cleanup: restore original stdout */
    dup2(stdoutfd, 1);

    /* and: close resources */
    fclose(redirect);
    close(stdoutfd);
    return 0;
}

static int checkOnlyOneLineWritten(const char* filename, const char* message)
{
    FILE* fp;
    char line[256];
    int count = 0;

    if ((fp = fopen(filename, "r")) == NULL) {
        nu_fail();
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strlen(line) - 1] = '\0'; /* remove LF */
        nu_assert_eq_int('I', line[0]);
        nu_assert_eq_str(message, &line[strlen(line) - strlen(message)]);
        count++;
    }
    nu_assert_eq_int(1, count);

    fclose(fp);
    return 0;
}

int main(int argc, char* argv[])
{
    setup();
    nu_run_test(test_multiLogger);
    cleanup();
    nu_report();
}
