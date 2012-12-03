/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef DATASET_H
#define DATASET_H

#include <stdio.h>

#define DATABLOCK_STRING_SIZE  124
#define MAGIC_STRING "VCell Data Dump"
#define VERSION_STRING  "2.0.1  "

typedef int int32;
typedef unsigned int uint32;

struct FileHeader {
	char   magicString[16];
	char   versionString[8];
	int32  numBlocks;
	int32   firstBlockOffset;
	int32   sizeX;
	int32   sizeY;
	int32   sizeZ;
};

struct DataBlock {
	char   varName[DATABLOCK_STRING_SIZE];
	int32   varType;
	int32   size;
	int32   dataOffset;
};

typedef enum {
	endian_not_set = -1,
	little_endian,
	big_endian
} Endian;

class SimulationExpression;

class DataSet
{
public:
	static void readHeader(FILE *fp, FileHeader *header);
	static void readDataBlock(FILE *fp, DataBlock *block);
	static void readDoubles(FILE *fp, double *data, int length);

	static bool isBigEndian();
	static void writeHeader(FILE *fp, FileHeader *header);
	static void writeDataBlock(FILE *fp, DataBlock *block);
	static void writeDoubles(FILE *fp, double *data, int length);

	static void read(char *filename, SimulationExpression *sim);
	static void write(char *filename, SimulationExpression *sim);
	
private:
	static Endian endian;
};

#endif
