#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define BLOCK_SIZE 1024
#define MAX_OPEN_FILES 8
#define MAX_BLOCK_COUNT 8096 

#define DIR_BLOCK_ENTRY_LENGTH 996
#define MAX_PATH_LENGTH 100
#define MAX_ENTRY_COUNT	40

#define FILE_BLOCK_DATA_SIZE 996
#define	NAME_SIZE 16

/*
	Block Types
		blockType:
				0 = empty
				1 = superBlock
				2 = dirBlock
				3 = fileBlock
*/

typedef struct superBlock
{
	int		blockType;
	int 		numBlocks;
	int		rootDirBlockNum;
	char 		blockBitmap[MAX_BLOCK_COUNT / 8];
} superBlock_t;

typedef struct dirBlock
{
	int		blockType;
	char 		name[NAME_SIZE];
	int		entryCount;
	int		continuationDirBlockNum;	//NOTE: this has been deprecated
	char		entries[DIR_BLOCK_ENTRY_LENGTH];
					//NOTE: The format for entry listings is as such
					//	name,blockType,blockNum;name,blockType,blockNum; ... 
} dirBlock_t;

typedef struct fileBlock
{
	int		blockType;
	char		name[NAME_SIZE];
	int 		internalFileSize;
	int		contBlockNum;
	char		data[FILE_BLOCK_DATA_SIZE];
} fileBlock_t;

/*
	Objects
*/

typedef struct blockLink
{
	int 			blockNum;
	struct blockLink	*nextLink;
} blockLink_t;

typedef struct
{
	char		name[NAME_SIZE];
	int		curContentsPtr;
	int		fileSize;
	blockLink_t	*fileHeadLink;
} activeFile_t;

typedef struct directoryEntry
{
	char			name[NAME_SIZE];
	int			blockType;
	int			startingBlockNum;
	struct directoryEntry	*nextEntry;
} directoryEntry_t;

typedef struct
{
	char			name[NAME_SIZE];
	char			path[MAX_PATH_LENGTH];
	int			entryCount;
	directoryEntry_t	*entries;
} activeDir_t;

/*
	Controllers
*/

typedef struct
{
	FILE *vdisk;
	
} diskController_t;

typedef struct
{
	activeFile_t		*activeFiles[MAX_OPEN_FILES];
	char			activeFilesBitmap;
	diskController_t	*diskCont;
	activeDir_t		*activeDir;
	superBlock_t		*superBlock;
} fileSystem_t;
