#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ASCII  128
#define FLAG_FILE 1
#define FLAG_OUTPUT 2
#define FLAG_CHAR 3
#define FLAG_WORD 4
#define FLAG_LINE 5
#define FLAG_LONGEST_WORD 6
#define FLAG_LONGEST_LINE 7
#define FLAG_BATCH 8

typedef struct Word {
    char *content;          
    int frequency;          
    int initialPos;         
    struct Word *nextWord;  
    struct Word *prevWord; 
} Word;

typedef struct Line {
    char *content;         
    int frequency;         
    int initialPos;        
    struct Line *nextLine; 
    struct Line *prevLine; 
} Line;

typedef struct CharElement {
    int frequency;  
    int initialPos; 
} CharElement;

typedef struct {
    CharElement chars[MAX_ASCII];
    Word *words;
    Line *lines;
    int totalChars;
    int uniqueChars;
    int totalWords;
    int uniqueWords;
    int totalLines;
    int uniqueLines;
    char **longestWords;
    int longestWordLength;
	int longestWordCount;
    char **longestLines;
    int longestLineLength;
	int longestLineCount;
} FileStats;

void processCharacterStats(FILE *file, CharElement chars[], int *totalChars, int *uniqueChars) {
	for(int i = 0; i < MAX_ASCII; i++) {
		chars[i].frequency = 0;
		chars[i].initialPos = -1;
	}

	int position = 0;
	*totalChars = 0;
	*uniqueChars = 0;

	int currentChar;
	while((currentChar = fgetc(file)) != EOF) {
		(*totalChars) += 1;
		if(currentChar >= 0 && currentChar < MAX_ASCII) {
			if(chars[currentChar].frequency == 0) {
				chars[currentChar].initialPos = position;
				(*uniqueChars) += 1;
			}

			chars[currentChar].frequency += 1;
		}

		position += 1;
	}

	fseek(file, 0, SEEK_SET);
}

void processWordStats(FILE *file, Word **wordList, int *totalWords, int *uniqueWords) {
	if (*wordList) return;
	char buffer[256];
    *totalWords = 0;
    *uniqueWords = 0;
    int wordIndex = 0;

    while (fscanf(file, " %255s", buffer) == 1) {
        (*totalWords) += 1;
        Word *foundWord = NULL;
        Word *prevWord = NULL;
        Word *current = *wordList;

        while (current != NULL) {
            if (strcmp(current->content, buffer) == 0) {
                foundWord = current;
                break; 
            } else if (strcmp(current->content, buffer) > 0) {
                break; 
            }
            prevWord = current;
            current = current->nextWord;
        }

        if (foundWord) {
            foundWord->frequency += 1;
        }
		else {
            Word *newWord = (Word *)malloc(sizeof(Word));
            newWord->content = strdup(buffer);
            newWord->frequency = 1;
            newWord->initialPos = wordIndex; 
            newWord->nextWord = current;

			if (*wordList == NULL) {
				*wordList = newWord;
			} else {
				if (prevWord == NULL) {
					*wordList = newWord; 
				} else {
					prevWord->nextWord = newWord; 
				}

				if (current != NULL) {
					current->prevWord = newWord;
				}
				newWord->prevWord = prevWord;
			}

            (*uniqueWords) += 1;
        }
        wordIndex++; 
    }    
	
	fseek(file, 0, SEEK_SET); 
}

void processLineStats(FILE *file, Line **lineList, int *totalLines, int *uniqueLines) {
	if (*lineList) return;
	char buffer[256];
    int linePosition = 0;
    *totalLines = 0;
    *uniqueLines = 0;

    while (fgets(buffer, 256, file) != NULL) {
        (*totalLines) += 1;

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        Line *foundLine = NULL;
        Line *prevLine = NULL;
        Line *current = *lineList;

        while (current != NULL && (foundLine == NULL)) {
            if (strcmp(current->content, buffer) == 0) {
                foundLine = current;
            } else if (strcmp(current->content, buffer) > 0) {
                break; 
            }
            prevLine = current;
            current = current->nextLine;
        }

        if (foundLine) {
            foundLine->frequency += 1;
        } else {
            Line *newLine = (Line *)malloc(sizeof(Line));
            if (newLine == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                break;
            }
            newLine->content = strdup(buffer);
            newLine->frequency = 1;
            newLine->initialPos = linePosition;
            newLine->nextLine = current; 

            if (prevLine == NULL) {
                *lineList = newLine; 
            } else {
                prevLine->nextLine = newLine; 
            }

            if (current != NULL) {
                current->prevLine = newLine; 
            }
            newLine->prevLine = prevLine; 

            (*uniqueLines) += 1;
        }
        linePosition++; 
    }

    fseek(file, 0, SEEK_SET); 
}

void findLongestWord(Word *wordList, char ***longestWords, int *longestWordLength, int *longestWordCount) {
	// printf("%p\n", wordList);
    *longestWordLength = 0;
    int count = 0;
    Word *current = wordList;

    while (current != NULL) {
        int length = strlen(current->content);
		// printf("%d\n", length);
        if (length > *longestWordLength) {
            *longestWordLength = length;
            count = 1; 
        } else if (length == *longestWordLength) {
            count++;
        }
        current = current->nextWord;
    }
	// printf("%d\n", *longestWordLength);

    
    *longestWords = (char **)malloc(count * sizeof(char *));
    if (*longestWords == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    count = 0; 
    current = wordList;
    
    while (current != NULL) {
        if (strlen(current->content) == *longestWordLength) {
            (*longestWords)[count++] = current->content;
        }
        current = current->nextWord;
    }
	*longestWordCount = count;
}

void findLongestLine(Line *lineList, char ***longestLines, int *longestLineLength, int *longestLineCount) {
	*longestLineLength = 0;
	int count = 0;

	Line *current = lineList;
	while (current != NULL) {
		int length = strlen(current->content);
		if(length > *longestLineLength) {
			*longestLineLength = length;
			count = 1;
		}
		else if(length == *longestLineLength) {
			count += 1;
		}
		current = current->nextLine;
	}

	*longestLines = (char **)malloc(count * sizeof(char *));
	if(*longestLines == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		return;
	}

	count = 0;
	current = lineList;
	while (current != NULL) {
		if(strlen(current->content) == *longestLineLength) {
			(*longestLines)[count++] = current->content;
		}
		current = current->nextLine;
	}
	*longestLineCount = count;
}

void analyzeFile(const char *inputFilePath, FileStats *stats, int flags[]){
	FILE *inputFile = fopen(inputFilePath, "r");
	if (inputFile == NULL){
		fprintf(stderr, "ERROR: Can't open input file\n");
    	return;
    }

    memset(stats, 0, sizeof(FileStats));

    if(flags[FLAG_CHAR - 3]) {
    	processCharacterStats(inputFile, stats->chars, &stats->totalChars, &stats->uniqueChars);
    	fseek(inputFile, 0, SEEK_SET);
    }

    if(flags[FLAG_WORD - 3]) {
      	processWordStats(inputFile, &stats->words, &stats->totalWords, &stats->uniqueWords);
       	fseek(inputFile, 0, SEEK_SET);
    }

    if(flags[FLAG_LINE - 3]) {
      	processLineStats(inputFile, &stats->lines, &stats->totalLines, &stats->uniqueLines);
      	fseek(inputFile, 0, SEEK_SET);
    }

    if(flags[FLAG_LONGEST_WORD - 3]) {
      	processWordStats(inputFile, &stats->words, &stats->totalWords, &stats->uniqueWords);
      	findLongestWord(stats->words, &stats->longestWords, &stats->longestWordLength, &stats->longestWordCount);
    }

    if(flags[FLAG_LONGEST_LINE - 3]) {
      	processLineStats(inputFile, &stats->lines, &stats->totalLines, &stats->uniqueLines);
      	findLongestLine(stats->lines, &stats->longestLines, &stats->longestLineLength, &stats->longestLineCount);
    }

    fclose(inputFile);
}

void outputResults(const FileStats *stats, FILE *stream, int *flags) {
	if(flags[FLAG_CHAR - 3]) {
    	fprintf(stream, "Total Number of Chars = %d\n", stats->totalChars);
    	fprintf(stream, "Total Unique Chars = %d\n\n", stats->uniqueChars);
    	for (int i = 0; i < MAX_ASCII; i++) {
        	if (stats->chars[i].frequency > 0) { // Add this check
            	fprintf(stream, "Ascii Value: %d, Char: ", i);
            	// if (i == 10) { // Specifically for newline
                // 	fprintf(stream, "\n");
            	// } else if (i > 31 && i < 127) {
                	fprintf(stream, "%c", (char)i);
            	//}
            	fprintf(stream, ", Count: %d, Initial Position: %d\n", stats->chars[i].frequency, stats->chars[i].initialPos);
        	}
    	}

		if (flags[FLAG_WORD-3] || flags[FLAG_LINE-3] || flags[FLAG_LONGEST_WORD-3] || flags[FLAG_LONGEST_LINE-3]) {
			fprintf(stream, "\n"); 
		}
	}

	if(flags[FLAG_WORD - 3]) {
		fprintf(stream, "Total Number of Words: %d\n", stats->totalWords);
		fprintf(stream, "Total Unique Words: %d\n", stats->uniqueWords);
		fprintf(stream, "\n");

		Word *currentWord = stats->words;
		while (currentWord != NULL) {
			fprintf(stream, "Word: %s, Freq: %d, Initial Position: %d\n", currentWord->content, currentWord->frequency, currentWord->initialPos);
			currentWord = currentWord->nextWord;
		}
		if (flags[FLAG_LINE-3] || flags[FLAG_LONGEST_WORD-3] || flags[FLAG_LONGEST_LINE-3]) {
			fprintf(stream, "\n"); 
		}
	}

	if(flags[FLAG_LINE - 3]) {
		fprintf(stream, "Total Number of Lines: %d\n", stats->totalLines);
		fprintf(stream, "Total Unique Lines: %d\n", stats->uniqueLines);
		fprintf(stream, "\n");

	
		Line *currentLine = stats->lines;
		while (currentLine != NULL) {
			fprintf(stream, "Line: %s, Freq: %d, Initial Position: %d\n", currentLine->content, currentLine->frequency, currentLine->initialPos);
			currentLine = currentLine->nextLine;
		}
		if (flags[FLAG_LONGEST_WORD-3] || flags[FLAG_LONGEST_LINE-3]) {
			fprintf(stream, "\n"); 
		}
	}

	if (flags[FLAG_LONGEST_WORD - 3]) {
		fprintf(stream, "Longest Word is %d characters long:\n", stats->longestWordLength);

	    for (int i = 0; i < stats->longestWordCount; i++) {
	    	fprintf(stream, "\t%s\n", stats->longestWords[i]);
	    }
		if (flags[FLAG_LONGEST_LINE-3]) {
			fprintf(stream, "\n"); 
		}
	}

	if (flags[FLAG_LONGEST_LINE - 3]) {
		fprintf(stream, "Longest Line is %d characters long:\n", stats->longestLineLength);
	    for (int i = 0; i < stats->longestLineCount; i++) {
	    	fprintf(stream, "\t%s\n", stats->longestLines[i]);
	     }
	     
	}
}

void freeFileStats(FileStats *stats) {
	Word *currentWord = stats->words;
	while (currentWord != NULL) {
		Word *nextWord = currentWord->nextWord;
	    free(currentWord->content);
	    free(currentWord);
	    currentWord = nextWord;
	}

	Line *currentLine = stats->lines;
	while (currentLine != NULL) {
		Line *nextLine = currentLine->nextLine;
	    free(currentLine->content);
    	free(currentLine);
	    currentLine = nextLine;
	}
	
	if (stats->longestWords) {
		free(stats->longestWords);
	}
	if (stats->longestLines) {
		free(stats->longestLines);
	}
}

int main(int argc, char *argv[]) {
	if(argc < 3) {
		printf("USAGE:\n\t./MADCounter -f <input file> -o <output file> -c -w -l -Lw -Ll\n\t\tOR\n\t./MADCounter -B <batch file>\n");
		return 1;
	}

	FILE *fp;
    char buffer[256];

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-B") == 0) {
            if (i + 1 >= argc) {
                printf("ERROR: Batch file name not provided\n");
                exit(1);
            }

            char *batchFilename = argv[i + 1];
            fp = fopen(batchFilename, "r");
            if (fp == NULL) {
                printf("ERROR: Can't open batch file\n");
                exit(1);
            }

            fseek(fp, 0, SEEK_END);
            if (ftell(fp) == 0) {
                printf("ERROR: Batch File Empty\n");
                fclose(fp);
                exit(1);
            }

            rewind(fp);

            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                buffer[strcspn(buffer, "\n")] = '\0';

                int maxArgs = 20; 
                char *myargs[maxArgs];
                myargs[0] = strdup("./MADCounter"); 
                int arg = 1;

                char *nextWord = strtok(buffer, " ");
                while (nextWord != NULL && arg < maxArgs - 1) {
                    myargs[arg++] = strdup(nextWord);
                    nextWord = strtok(NULL, " ");
                }
                myargs[arg] = NULL; 

                pid_t rc = fork();
                if (rc < 0) {
                    fprintf(stderr, "fork failed\n");
                    exit(1);
                } else if (rc == 0) {
                    execvp(myargs[0], myargs);
                    fprintf(stderr, "execvp failed\n");
                    exit(1);
                } else {
                    int status;
                    waitpid(rc, &status, 0);
                }

                for (int j = 0; j < arg; j++) {
                    free(myargs[j]);
                }
            }

            fclose(fp);
            exit(0); 
        }
    }

	char *inputFilePath = NULL;
	char *outputFilePath = NULL;
	int flags[7] = {0};
	int last = -1;

	for(int i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-f") == 0) {
        	if(i + 1 < argc && argv[i+1][0] != '-') {
            	inputFilePath = argv[++i];
        	} else {
           		printf("ERROR: No Input File Provided\n");
           		return 1;
        	}
    	}
   		else if(strcmp(argv[i], "-o") == 0) {
        	if(i + 1 < argc && argv[i+1][0] != '-') {
            	outputFilePath = argv[++i];
        	} else {
            	printf("ERROR: No Output File Provided\n");
            	return 1;
        	}
    	}
		else if (strcmp(argv[i], "-c") == 0) {
			flags[FLAG_CHAR - 3] = 1;
			last = i;
		}
		else if (strcmp(argv[i], "-w") == 0) {
		    flags[FLAG_WORD - 3] = 1;
			last = i;
		}
		else if (strcmp(argv[i], "-l") == 0) {
			flags[FLAG_LINE - 3] = 1;
			last = i;
		}
		else if (strcmp(argv[i], "-Lw") == 0) {
			flags[FLAG_LONGEST_WORD - 3] = 1;
			last = i;
		}
		else if (strcmp(argv[i], "-Ll") == 0) {
			flags[FLAG_LONGEST_LINE - 3] = 1;
			last = i;
		} 
		else {
			printf("ERROR: Invalid Flag Types\n");
		    return 1;
		}
	}

	if(inputFilePath == NULL) {
		printf("ERROR: No Input File Provided\n");
		return 1;
	}

	FILE *inputFile = fopen(inputFilePath, "r");
	if(inputFile == NULL) {
		printf("ERROR: Can't open input file\n");
		return 1;
	}

	if(fgetc(inputFile) == EOF) {
		printf("ERROR: Input File Empty\n");
		fclose(inputFile);
		return 1;
	}
	rewind(inputFile);

	FileStats stats;
	memset(&stats, 0, sizeof(FileStats));
	
	analyzeFile(inputFilePath, &stats, flags);
	
	fclose(inputFile);

	FILE *stream;
	
	if (outputFilePath) {
	    stream = fopen(outputFilePath, "w");
	    if (stream == NULL) {
	        printf("ERROR: Can't open output file\n");
	        freeFileStats(&stats); 
	        return 1;
	    }
	}
	else {
		stream = stdout;
	}

	for(int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0) {
			fprintf(stream, "Total Number of Chars = %d\n", stats.totalChars);
			fprintf(stream, "Total Unique Chars = %d\n\n", stats.uniqueChars);
			for (int i = 0; i < MAX_ASCII; i++) {
				if (stats.chars[i].frequency > 0) { // Add this check
					fprintf(stream, "Ascii Value: %d, Char: ", i);
					fprintf(stream, "%c", (char)i);
					fprintf(stream, ", Count: %d, Initial Position: %d\n", stats.chars[i].frequency, stats.chars[i].initialPos);
				}
			}

			if (last != i) {
				fprintf(stream, "\n"); 
			}
		}
		else if (strcmp(argv[i], "-w") == 0) {
			fprintf(stream, "Total Number of Words: %d\n", stats.totalWords);
			fprintf(stream, "Total Unique Words: %d\n", stats.uniqueWords);
			fprintf(stream, "\n");

			Word *currentWord = stats.words;
			while (currentWord != NULL) {
				fprintf(stream, "Word: %s, Freq: %d, Initial Position: %d\n", currentWord->content, currentWord->frequency, currentWord->initialPos);
				currentWord = currentWord->nextWord;
			}
			if (last != i) {
				fprintf(stream, "\n"); 
			}
		    
		}
		else if (strcmp(argv[i], "-l") == 0) {
			fprintf(stream, "Total Number of Lines: %d\n", stats.totalLines);
			fprintf(stream, "Total Unique Lines: %d\n", stats.uniqueLines);
			fprintf(stream, "\n");

	
			Line *currentLine = stats.lines;
			while (currentLine != NULL) {
				fprintf(stream, "Line: %s, Freq: %d, Initial Position: %d\n", currentLine->content, currentLine->frequency, currentLine->initialPos);
				currentLine = currentLine->nextLine;
			}
		
			if (last != i) {
				fprintf(stream, "\n"); 
			}
		}
		else if (strcmp(argv[i], "-Lw") == 0) {
			fprintf(stream, "Longest Word is %d characters long:\n", stats.longestWordLength);

	    	for (int i = 0; i < stats.longestWordCount; i++) {
	    		fprintf(stream, "\t%s\n", stats.longestWords[i]);
	    	}
			if (last != i) {
				fprintf(stream, "\n"); 
			}
		}
		else if (strcmp(argv[i], "-Ll") == 0) {
			fprintf(stream, "Longest Line is %d characters long:\n", stats.longestLineLength);
	    	for (int i = 0; i < stats.longestLineCount; i++) {
	    		fprintf(stream, "\t%s\n", stats.longestLines[i]);
	     	}
			if (last != i) {
				fprintf(stream, "\n"); 
			}
		}
	}

    

	if (stream != stdout)
	    fclose(stream);
	
	freeFileStats(&stats);
	return 0;
}
    
