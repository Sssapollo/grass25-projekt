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
    "   -e, --endianness N:uint8_t            Endianness: 0=Little-Endian, 1=Big-Endian(default:0)\n"
    "   -s, --latency-scrambling N:uint32_t    Latency for Address Scrambling in clock cycles(default:0)\n"
    "   -n, --latency-encrypt N:uint32_t       Latency for Encryption/Decryption in clock cycles(default:0)\n"
    "   -m, --latency-memory-access N:uint32_t Latency for Memory Access in clock cycles(default:0)\n"
    "   -d, --seed N:uint32_t                  Seed for the pRNG\n"
    "   -h,--help: flag                Show the help message\n";

const char* help_msg =
    "Positional arguments:\n"
    "  <file>  The path to input file:list of requests\n"
    "\n"
    "Optional arguments:\n"
    "  -c, --cycles N                Number of cycles (uint32_t, default: 1)\n"
    "  -t, --tf PATH                 Path to the TraceFile (string)\n"
    "  -e, --endianness N            Endianness (uint8_t): 0=Little-Endian, 1=Big-Endian\n"
    "  -s, --latency-scrambling N    Latency for Address Scrambling (uint32_t)\n"
    "  -n, --latency-encrypt N       Latency for Encryption/Decryption (uint32_t)\n"
    "  -m, --latency-memory-access N Latency for Memory Access (uint32_t)\n"
    "  -d, --seed N                  Seed for pRNG (uint32_t)\n"
    "  -h, --help                    Show this help message\n";
void print_usage(const char* progname) {
    fprintf(stderr, usage_msg, progname);
}

void print_help(const char* progname) {
    printf(usage_msg, progname);
    printf("\n%s", help_msg);
}


int parse_cli(int argc, char *argv[], uint32_t *cycles, char **tracefile, char **input_file,
              uint8_t *endianness, uint32_t *latency_scrambling, uint32_t *latency_encrypt,
              uint32_t *latency_memory_access, uint32_t *seed) {  
    static struct option long_options[] = {
        {"cycles", required_argument, 0, 'c'},
        {"tf", required_argument, 0, 't'},
        {"endianness", required_argument, 0, 'e'},
        {"latency-scrambling", required_argument, 0, 's'},
        {"latency-encrypt", required_argument, 0, 'n'},
        {"latency-memory-access", required_argument, 0, 'm'},
        {"seed", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int opt;
    while ((opt = getopt_long(argc,argv,"c:t:e:s:n:m:d:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                *cycles = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 't':
                *tracefile = optarg;
                break;
            case 'e':
                *endianness = (uint8_t)strtoul(optarg, NULL, 10);
                if (*endianness != 0 && *endianness != 1) {
                    fprintf(stderr, "Invalid endianness value. Use 0 for Little-Endian or 1 for Big-Endian.\n");
                    return EXIT_FAILURE;
                }
                break;
            case 's':
                *latency_scrambling = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 'n':
                *latency_encrypt = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 'm':
                *latency_memory_access = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 'd':
                *seed = (uint32_t)strtoul(optarg, NULL, 10);
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
    static uint8_t endianness = 0; // Default: Little-Endian
    static uint32_t latency_scrambling = 0; // Default: 0
    static uint32_t latency_encrypt = 0; // Default: 0
    static uint32_t latency_memory_access = 0; // Default: 0
    static uint32_t seed = 0; // Default: 0

    int result = parse_cli(argc, argv, &cycles , &tracefile, &input_file,
                           &endianness, &latency_scrambling, &latency_encrypt,
                           &latency_memory_access, &seed);
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
    //TODO

    return 0;
}