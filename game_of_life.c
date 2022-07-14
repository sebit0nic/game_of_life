//-----------------------------------------------------------------------------
// game_of_life.c
//
// Implementation of the classic game of life "zero player game".
//
// Author: Sebastian Lackner
//-----------------------------------------------------------------------------
//

//================
/// INCLUDES
//================
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

//================
/// DEFINES
//================
#define STANDARD_WIDTH 10
#define STANDARD_HEIGHT 10
#define USAGE_PROMPT "Usage: ./gol [-f <filename>]\n"
#define INFO_DEFAULT_FILE "-> Info: Using standard configuration file \"%s\"\n"
#define ERROR_NO_FILE "-> Error: Configuration file \"%s\" does not exist!\n"
#define DEFAULT_CONFIG_PATH "default.txt"

//================
/// ENUMS
//================
typedef enum _ProgramReturn_ 
{ 
  OK, 
  ERROR 
} ProgramReturn;

//================
/// STRUCTS
//================
typedef struct _Neighbour_
{
  int offset_y;
  int offset_x;
} Neighbour;

typedef struct _Cell_
{
  char current_value;
  char new_value;
} Cell;


//------------------------------------------------------------------------------
///
/// Checks if the command line parameters are correct.
///
/// @param argc - the argument count
/// @param argv - a list of command strings
/// @param file_path - path to the config file
///
/// @return 0 if parameters are valid, otherwise a value > 1
//
int checkParams(int argc, char *argv[], char **file_path)
{
  if (argc == 1)
  {
    printf(INFO_DEFAULT_FILE, DEFAULT_CONFIG_PATH);
    *file_path = (char*) calloc(sizeof(char), sizeof(DEFAULT_CONFIG_PATH));
    if (*file_path == NULL)
    {
      return ERROR;
    }
    strcpy(*file_path, DEFAULT_CONFIG_PATH);
  }
  else if (argc == 3)
  {
    if (!strcmp(argv[1], "-f"))
    { 
      printf("-> Using configuration file: %s\n", argv[2]);
      *file_path = (char*) calloc(sizeof(char), strlen(argv[2]) + 1);
      if (*file_path == NULL)
      {
        return ERROR;
      }
      strcpy(*file_path, argv[2]);
    }
    else
    {
      printf(USAGE_PROMPT);
      return ERROR;
    }
  }
  else
  {
    printf(USAGE_PROMPT);
    return ERROR;
  }
  return OK;
}

//------------------------------------------------------------------------------
///
/// Checks if the config file is valid.
///
/// @param config_file - the actual loaded config file
/// @param file_path - path to the config file
/// @param board_height - pointer to the height of the board
/// @param board_width - pointer to the width of the board
///
/// @return 0 if file is valid, otherwise a value > 1
//
int checkConfigFile(FILE **config_file, char *file_path, uint8_t *board_height, uint8_t *board_width)
{
  *config_file = fopen(file_path, "rb");

  if (*config_file == NULL)
  {
    printf(ERROR_NO_FILE, file_path);
    return ERROR;
  }

  int last_column_count = 0;
  int current_column_count = 0;
  int row_count = 0;
  while (!feof(*config_file))
  {
    char current_char = fgetc(*config_file);
    current_column_count++;
    if (current_char == '\n')
    {
      (last_column_count == 0) ? (last_column_count = current_column_count) : (last_column_count);
      if (last_column_count != current_column_count)
      {
        printf("-> Error: Inconsistent column count detected!\n");
        fclose(*config_file);
        return ERROR;
      }
      row_count++;
      current_column_count = 0;
    }
    else if (current_char == EOF)
    {
      if (last_column_count != current_column_count)
      {
        printf("-> Error: Inconsistent column count detected!\n");
        fclose(*config_file);
        return ERROR;
      }
      break;
    }
    else if (current_char != '.' && current_char != '#')
    {
      printf("-> Error: Invalid char \"%c\" detected!\n", current_char);
      fclose(*config_file);
      return ERROR;
    }
    
  }

  *board_width = last_column_count - 1;
  *board_height = row_count + 1;

  printf("-> Info: Rows = %d, Columns = %d\n", *board_height, *board_width);
  return OK;
}

//------------------------------------------------------------------------------
///
/// Fills the board and checks if board is valid.
///
/// @param config_file - the actual loaded config file
/// @param board - a 2D representation of the board
/// @param board_height - the height of the board
/// @param board_width - the width of the board
///
/// @return 0 if board is valid, otherwise a value > 1
//
int fillBoard(FILE *config_file, Cell ***board, uint8_t board_height, uint8_t board_width)
{
  rewind(config_file);
  *board = (Cell**) calloc(board_height, sizeof(Cell*));
  if (*board == NULL)
  {
    return ERROR;
  }
  for (uint8_t row = 0; row < board_height; row++)
  {
    (*board)[row] = (Cell*) calloc(board_width, sizeof(Cell));
    if ((*board)[row] == NULL)
    {
      return ERROR;
    }
  }

  for (uint8_t row = 0; row < board_height; row++)
  {
    char current_char;
    for (uint8_t column = 0; column < board_width; column++)
    {
      current_char = fgetc(config_file);
      (*board)[row][column].current_value = current_char;
      (*board)[row][column].new_value = current_char;
    }
    current_char = fgetc(config_file);
  }

  return OK;
}

//------------------------------------------------------------------------------
///
/// Updates all the cells of the board for the next step
///
/// @param board - a 2D representation of the board
/// @param board_height - the height of the board
/// @param board_width - the width of the board
//
void updateBoard(Cell **board, int board_height, int board_width)
{
  Neighbour neighbour[8] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1} };

  for (size_t row = 0; row < board_height; row++)
  {
    for (size_t column = 0; column < board_width; column++)
    {
      size_t neighbour_count = 0;
      
      // Checking all 8 neighbours
      for (size_t count = 0; count < 8; count++)
      {
        if (row + neighbour[count].offset_y >= board_height || column + neighbour[count].offset_x >= board_width)
        {
          continue;
        }
        if (board[row + neighbour[count].offset_y][column + neighbour[count].offset_x].current_value == '#')
        {
          neighbour_count++;
        }
      }
      // We are on a live cell and we have two or three live neighbours = survive
      if (board[row][column].current_value == '#' && (neighbour_count == 2 || neighbour_count == 3))
      {
        board[row][column].new_value = '#';
      }
      // We are on a dead cell and we have exactly three neighbours = become alive
      else if (board[row][column].current_value == '.' && neighbour_count == 3)
      {
        board[row][column].new_value = '#';
      }
      // Otherwise the cell dies/stays dead
      else
      {
        board[row][column].new_value = '.';
      }
    }
  }

  // Persist values
  for (size_t row = 0; row < board_height; row++)
  {
    for (size_t column = 0; column < board_width; column++)
    {
      board[row][column].current_value = board[row][column].new_value;
    }
  }
}

//------------------------------------------------------------------------------
///
/// Prints the whole board to the console
///
/// @param board - a 2D representation of the board
/// @param board_height - the height of the board
/// @param board_width - the width of the board
//
void printBoard(Cell **board, int board_height, int board_width)
{
  for (uint8_t column = 0; column < board_width; column++)
  {
    printf("═");
  }
  printf("╗\n");
  for (uint8_t row = 0; row < board_height; row++)
  {
    printf("║");
    for (uint8_t column = 0; column < board_width; column++)
    {
      if (board[row][column].current_value == '#')
      {
        printf("■");
      }
      if (board[row][column].current_value == '.')
      {
        printf("·");
      }
    }
    printf("║\n");
  }
  printf("╚");
  for (uint8_t column = 0; column < board_width; column++)
  {
    printf("═");
  }
  printf("╝\n");
}

//------------------------------------------------------------------------------
///
/// Do all the pre-checks and then run the game of life simulation.
///
/// @param argc - the argument count
/// @param argv - a list of command strings
///
/// @return 0 if game was run successfully, otherwise a value > 0
//
int run(int argc, char *argv[])
{
  FILE *config_file = NULL;
  char *file_path = NULL;
  Cell **board = NULL;
  uint8_t board_height = 0;
  uint8_t board_width = 0;
  size_t step = 0;

  if (checkParams(argc, argv, &file_path))
  {
    return ERROR;
  }
  if (checkConfigFile(&config_file, file_path, &board_height, &board_width))
  {
    return ERROR;
  }
  if (fillBoard(config_file, &board, board_height, board_width))
  {
    return ERROR;
  }
  
  sleep(1);
  printf("\n============ GOL - Game Of Life ============\n");
  while(1)
  {
    printf("Step: %zu\n╔", step);
    printBoard(board, board_height, board_width);
    updateBoard(board, board_height, board_width);
    step++;
    sleep(1);
  }

  return OK;
}

//------------------------------------------------------------------------------
///
/// Main entry point of the program.
///
/// @param argc - the argument count
/// @param argv - a list of command strings
///
/// @return 0 if game was run successfully, otherwise a value > 0
//
int main(int argc, char *argv[])
{
  return run(argc, argv);
}