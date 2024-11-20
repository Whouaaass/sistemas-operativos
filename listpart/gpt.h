/**
* @file
* @author Erwin Meza Vega <emezav@unicauca.edu.co>
* @brief Definiciones para discos inicializados con esquema GPT
* @copyright MIT License
*/

#ifndef GPT_H
#define GPT_H

#include "mbr.h"

/**
* @brief GUID
* @see https://uefi.org/specs/UEFI/2.10/Apx_A_GUID_and_Time_Formats.html
*/
typedef struct {
	unsigned int time_lo; /*!< Low field of the timestamp */
	unsigned short time_mid; /*!< Middle field of the timestamp */
	unsigned short time_hi_and_version; /*!< High field of the timestamp and version */
	unsigned char clock_seq_hi_and_reserved; /*!< High field of the clock sequence */
	unsigned char clock_seq_lo; /*!< Low field of the clock sequence */
	unsigned char node[6]; /*!< Spatially unique node identifier */
}__attribute__((packed))guid;

// 512
/** @brief GPT Partition Table Header.*/
typedef struct {
	unsigned long long signature;
	unsigned int revision;
	unsigned int header_size; /* !< Size in bytes of the GPT Header */
	unsigned int header_crc32; /* !< Checksum */
	unsigned int reserved;
	unsigned long long my_lba; /* !< The LBA that contains this data structure */
	unsigned long long alternate_lba; /* !< The other LBA of the GPT header */
	unsigned long long first_lba; /* !< The first usable LBA */
	unsigned long long last_lba; /* !< The last usable LBA*/
	guid disk_guid; /* !< GUID to uniquely identify the disk*/
	unsigned long long partition_entry_lba; /* !< The starting lba of the */
	unsigned int npartition_entries;
	unsigned int partition_entry_size; /* !< this should be a value 128*2n */
	unsigned int partition_entry_arr_crc32; /* !< this */
	unsigned char reserved_end[420];
}__attribute__((packed)) gpt_header;

/**
* @brief GPT Partition Entry
*/
typedef struct {
	guid partition_type_guid;
	guid unique_partition_guid;
	unsigned long long starting_lba;
	unsigned long long ending_lba;
	unsigned long long attributes;
	unsigned char partition_name[72];
}__attribute__((packed)) gpt_partition_descriptor;

/**
* @brief GPT Partition type
*/
typedef struct {
	const char * os; /*!< Operating system */
	const char * description; /*!< Description */
	const char * guid; /*!< GUID */
} gpt_partition_type;

/**
* @brief Text description of a GPT partition type
* @param type Partition type reported in MBR
* @param buf String buffer to store the text description
*/
const gpt_partition_type* get_gpt_partition_type(char * guid_str);

/**
* @brief Decodes a two-byte encoded partition name
* @param name two-byte encoded partition name
*/
char * gpt_decode_partition_name(char name[72]);

/**
* @brief Checks if a bootsector is Protective MBR.
* @param boot_record Bootsector read in memory]
* @return 1 If the bootsector is a Protective MBR, 0 otherwise.
*/
int is_protective_mbr(mbr * boot_record);

/**
* @brief Checks if a GPT header is valid.
* @param hdr Pointer to the GPT header
* @return 1 of hdr is a valid GPT header, 0 otherwise.
*/
int is_valid_gpt_header(gpt_header * hdr);


/**
* @brief Checks if the GPT partition descriptor is null (not used)
* @param desc Descriptor
* @return 1 if the descriptor is null (partition_type_guid = 0), 0 otherwise.
*/
int is_null_descriptor(gpt_partition_descriptor * desc);


/**
* @brief Creates a human-readable representation of a GUID
* @param buf Buffer containing the GUID
* @return New string with the text representation of the GUID
*/
char * guid_to_str(guid * buf);

#endif
