/**
 * @file
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @author Fredy Anaya <fredyanaya@unicauca.edu.co>
 * @brief Listpart particiones de discos duros MBR/GPT
 * @copyright MIT License
 */

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mbr.h"
#include "gpt.h"

/** @brief cantidad de bytes en un sector */
#define SECTOR_SIZE 512U

/**
 * @brief prints the usage of the command
 */
void usage();

/**
 * @brief Hex dumps a buffer
 * @param buf Pointer to buffer
 * @param size Buffer size
 */
void hex_dump(char *buf, size_t size);

/**
 * @brief ASCII dumps a buffer
 * @param buf Pointer to buffer
 * @param size Buffer size
 */
void ascii_dump(char *buf, size_t size);

/**
 * @brief Reads a sector from a disk
 *
 * @param disk number of disk to read
 * @param lba Sector to read ( 0 - amount of LBA sectors on disk)
 * @param buf Buffer to read sector into
 * @return int 1 on success, 0 on failure
 */
int read_lba_sector(char *disk, unsigned long long lba, char buf[SECTOR_SIZE]);

/**
 * @brief Prints the MBR table
 * @param mbr MBR table to print
 */
void print_mbr_table(mbr *mbr);

/**
 * @brief Prints the GPT header
 * @param header GPT header to print
 */
void print_gpt_header(gpt_header *header);

/**
 * @brief Routine in charge of list all the partitions info of a disk
 * @param device_route Route where the device is located
 * @return 0 on success, otherwise on failure
 */
int list_partitions(char *device_route);

int main(int argc, char *argv[])
{
	int i;
	char *disk;
	// 1. validar los argumentos de la linea de comandos
	if (argc < 1)
	{
		usage();
		exit(EXIT_FAILURE);
	}

	for (i = 1; i < argc; i++)
	{
		printf("Listing partitions of %s\n", argv[i]);
		list_partitions(argv[i]);
	}

	return 0;
}

int list_partitions(char *disk_route)
{
	// 2. Leer el primer sector del disco especificado
	mbr boot_record;
	gpt_header header;

	if (read_lba_sector(disk_route, 0, (char *)&boot_record) == 0)
	{
		// 2.1. Si la lectura falla, imprimir el error en la terminal
		fprintf(stderr, "unable to open device %s\n", disk_route);
		return 1;
	}

	if (!is_mbr(&boot_record)) {
		fprintf(stderr, "the device is not mbr partitioned %s\n", disk_route);
		return 2;
	}

	// PRE: se pudo leer el primer sector del disco y se leyó el MBR
	// 3. Imprimir la tabla de particiones del MBR leido
	print_mbr_table(&boot_record);
	// 4. Si el esquema de particionado no es protected MBR: terminar.
	if (!is_protective_mbr(&boot_record))
		return 0;
	// PRE: El esquema de particionado es GPT
	// 5. Imprimir la tabla GPT:
	// 5.1. Leer el segundo sector del disco (encabezado de la tabla de particiones GPT PTHDR)
	if (read_lba_sector(disk_route, 1, (char *)&header) == 0)
	{
		// 5.1.1. Si la lectura falla, imprimir el error en la terminal
		fprintf(stderr, "unable to open device on gpt %s\n", disk_route);
		return 1;
	}

	if (!is_valid_gpt_header(&header))
	{
		fprintf(stderr, "the gpt header is not valid %s\n", disk_route);
		return 3;
	}
	unsigned int nsectors = header.partition_entry_size * header.npartition_entries / SECTOR_SIZE;
	print_gpt_header(&header);
	// En el PTHDR se encuentran la cantidad de descriptores de la tabla
	printf(
		"Start LBA    End LBA      Size          Type                             Partition Name\n"
		"------------ ------------ ------------- -------------------------------- ------------------------------\n");
	// 5.2. Repetir:
	for (int i = 0; i < nsectors; i++)
	{
		unsigned int sector = header.partition_entry_lba + i;
		gpt_partition_descriptor descs[4];
		// 5.2.1. Leer un sector que contiene descriptores de particion GPT
		if (read_lba_sector(disk_route, sector, (char *) &descs) == 0) 
		{
			fprintf(stderr, "unable to read sector %u\n", sector);
			return 4;
		}
		// 5.2.2. Para cada descriptor leido, imprimir su información
		for (int j = 0; j < 4; j++)
		{
			gpt_partition_descriptor *desc = &descs[j];

			if (is_null_descriptor(desc))
				continue;			

			const gpt_partition_type *pt = get_gpt_partition_type(guid_to_str(&desc->partition_type_guid));
			printf("%12lu %12lu %13lu %s %s\n",
				   desc->starting_lba,
				   desc->ending_lba,
				   (desc->ending_lba - desc->starting_lba + 1) * SECTOR_SIZE,
				   pt->description == NULL ? "Unknown" : pt->description,
				   gpt_decode_partition_name(desc->partition_name));
		}
	}
	printf("------------ ------------ ------------- -------------------------------- ------------------------------\n");
}

int read_lba_sector(char *disk, unsigned long long lba, char buf[SECTOR_SIZE])
{
	FILE *fp;

	// Abrir el disco en modo lectura
	fp = fopen(disk, "r");

	if (fp == NULL)
		return 0;

	// avanzar el apuntador de lectura
	if (fseek(fp, lba * SECTOR_SIZE, SEEK_SET))
		return 0;
	// leer en buffer
	fread(buf, 1, SECTOR_SIZE, fp);

	fclose(fp);

	return 1;
}

void print_mbr_table(mbr *mbr)
{
	printf("MBR table:\n");
	printf(
		"Start LBA    End LBA      Type\n"
		"------------ ------------ --------------------------------\n");
	for (int i = 0; i < 4; i++)
	{
		mbr_partition_descriptor *partition = &mbr->partition_table[i];
		if (partition->type == MBR_TYPE_UNUSED)
			continue;

		char partition_name[TYPE_NAME_LEN];
		mbr_partition_type(partition->type, partition_name);

		printf("%12u %12u %s\n",
			   partition->lba_start,
			   partition->lba_size - partition->lba_start,
			   partition_name);
	}
	printf("------------ ------------ --------------------------------\n");
}

void print_gpt_header(gpt_header *header)
{
	printf("GPT Header\n");
	printf("Revision: 0x%llx\n", header->signature);
	printf("First usable LBA: %llu\n", header->first_lba);
	printf("Last usable LBA: %llu\n", header->last_lba);
	printf("DISK GUID: %s\n", guid_to_str(&header->disk_guid));
	printf("Partition Entry LBA: %llu\n", header->partition_entry_lba);
	printf("Number of partition entries: %u\n", header->npartition_entries);
	printf("Size of partition entry: %u\n", header->partition_entry_size);
	printf("Total of partition table entry sectors: %u\n", (header->partition_entry_size * header->npartition_entries) / SECTOR_SIZE);
	printf("Size of partition descriptor: %u\n", header->partition_entry_size);
}

void ascii_dump(char *buf, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (buf[i] >= 0x20 && buf[i] < 0x7F)
		{
			printf("%c", buf[i]);
		}
		else
		{
			printf(".");
		}
	}
}

void hex_dump(char *buf, size_t size)
{
	int cols;
	cols = 0;
	for (size_t i = 0; i < size; i++)
	{
		printf("%02x ", buf[i] & 0xff);
		if (++cols % 16 == 0)
		{
			ascii_dump(&buf[cols - 16], 16);
			printf("\n");
		}
	}
}

void usage()
{
	fprintf(stderr, "Usage: listpart <device_1> [device_2 ...]\n");
}