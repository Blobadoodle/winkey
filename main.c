#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define MSDM "/sys/firmware/acpi/tables/MSDM"
#define SLIC "/sys/firmware/acpi/tables/SLIC"
#define VERSION "winkey v1.0.0\n"

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} sdt_header_t;
typedef struct {
    uint32_t version;
    uint32_t reserved;
    uint32_t data_type;
    uint32_t data_reserved;
    uint32_t data_length;
    char data[];
} license_t;
typedef struct {
    sdt_header_t h;
    license_t license;
} msdm_slic_t;


int main(int argc, char *argv[]) {
    bool verbose = false;

    if(argc > 1) { // TODO: better argument handling, this kinda sucks
        if(!strcmp(argv[1], "-v")) {
            printf(VERSION);
            return 0;
        } else if (!strcmp(argv[1], "-d")) {
            verbose = true;
        }
    }

    char *fn = MSDM;

    if(access(MSDM, F_OK)) { // Choose MSDM table if it exists, fallback to SLIC, or just fail if neither exist
        if(access(SLIC, F_OK)) {
            fprintf(stderr, "\nMSDM or SLIC table not present. Key does not exist\n");
            return 1;
        } else {
            if(verbose) printf("MSDM table not present, falling back to SLIC\n");
            fn = SLIC;
        }
    }
    FILE *file; // Read the base structure of the file
    file = fopen(fn, "rb");
    if(file == NULL) {
        fprintf(stderr, "\nError opening table. Try running as root\n");
        return 2;
    }
    msdm_slic_t table;
    fread(&table, sizeof(table), 1, file);

    size_t length = (size_t)table.license.data_length; // Read the key of the file
    char *key = malloc(sizeof(char) * (length+1));
    if(key == NULL) {
        fprintf(stderr, "\nFailed to allocate memory.\n");
        return 3;
    }
    fread(key, sizeof(char), length+1, file);
    key[length] = '\0';

    if(verbose) { // No one wants this, but why not
        printf("---Debug---\n"
               "Table Version : %d\n"
               "Key Type      : %d\n"
               "Key Length    : %d\n"
               "Key           : ",
               table.license.version,
               table.license.data_type,
               table.license.data_length
        );
    }
    printf("%s\n",key);

    free(key);
    fclose(file);
    return 0;
}