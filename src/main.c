#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


const char* usage_msg = 
    "Usage: %s [options] <file>\n   "
    "Options:\n"
    "   -c,--cycles N: uint32_t        Number of cycles (default: 1)\n"
    "   -t,--tf : String               Path to the TraceFile\n"
    "   -h,--help: flag                Show the help message\n";

const char* help_msg =
    "Positional arguments:\n"
    "  <file>  The path to input file:list of requests\n"
    "\n"
    "Optional arguments:\n"
    "  --cycles: uint32_t   The number of cycles (default: 1)\n"
    "  --tf: String          Path to the TraceFile\n"
    "  --help: flag          Show the help message\n";

void print_usage(const char* progname) {
    fprintf(stderr, usage_msg, progname);
}

void print_help(const char* progname) {
    printf(usage_msg, progname);
    printf("\n%s", help_msg);
}


int parse_cli(int argc, char *argv[], uint32_t *cycles, char **tracefile, char **input_file) {  
    static struct option long_options[] = {
        {"cycles", required_argument, 0, 'c'},
        {"tf", required_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int opt;
    while ((opt = getopt_long(argc,argv,"c:t:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                *cycles = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 't':
                *tracefile = optarg;
                break;
            case 'h':
                print_help(argv[0]);
                return EXIT_SUCCESS;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "No input file specified.\n");
        return 1;
    }
     while (optind < argc) {
        if (*input_file != NULL) {
            fprintf(stderr, "Too many input files specified.\n");
            return 1;
        }
        *input_file = argv[optind];
        optind++;
    }

    return EXIT_SUCCESS;
}


FILE* validate_and_open_read(const char *path) {
    FILE* file;
    if (!(file = fopen(path, "r"))) {
        return NULL;
    }

    struct stat statbuf;
    if (fstat(fileno(file), &statbuf)) {
        fclose(file);
        return NULL;
    }
    if (!S_ISREG(statbuf.st_mode) || statbuf.st_size <= 0) {
        fclose(file);
        return NULL;
    }
    return file;
}


int main(int argc, char** argv){

    static uint32_t cycles = 1;
    static char *tracefile = NULL;
    static char *input_file = NULL;

    int result = parse_cli(argc, argv, &cycles , &tracefile, &input_file);
    if (result != 0 || input_file == NULL) {
        return result;
    }


     FILE* file = validate_and_open_read(input_file);
    if (file == NULL) {
        fprintf(stderr, "Failed to open input file '%s'.\n", input_file);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* input = (char*)malloc(file_size + 1);
    if (input == NULL) {
        fprintf(stderr, "Failed to allocate memory for input file.\n");
        fclose(file);
        return 1;
    }
    fread(input, 1, file_size, file);
    fclose(file);
    input[file_size] = '\0';

    return 0;
}