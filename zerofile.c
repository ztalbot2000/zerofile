#include <sys/param.h>             // For system parameters
#include <sys/errno.h>             // For system calls and error numbers
#include <unistd.h>                // For getopt universal routines
#include <time.h>                  // For time functions
#include <stdio.h>                 // For standard io routines
#include <stdlib.h>                // For standard library functions
#include <string.h>                // For string functions
#include <signal.h>                // For simplified software signal facilities

typedef enum {FALSE, TRUE} Bool;

#define minAmount 256
#define DEFAULT_TMP_FILENAME "0slask0.zro"
#define DEFAULT_BLKSIZE 4096


char *tmpFilename = DEFAULT_TMP_FILENAME;
size_t blockSize = DEFAULT_BLKSIZE; // Amount of zeroes to write at each pass
Bool quiet = FALSE;                 // No progress report during zeroing, a bit faster
FILE *fid;

//***********************************************************************
//
// showHelp
//
// Description:
//     This procedure gives the correct syntax of the input command
//
// Parameters:
//    ( char * ) program - This programs name, usually argv[ 0 ]
//
// Returns:
//    ( void )
//***********************************************************************/
void showHelp( program )
char *program;
{
  system( "clear" );
  fprintf( stdout, "%s -q -b < blkdSize > [ tempFile ]\n\n", program );
  fprintf( stdout, "   Description: \n" );
  fprintf( stdout, "\n" );
  fprintf( stdout, "      A simple command line tool that fills FREE space on a disk with zeroes by\n" );
  fprintf( stdout, "      filling a temporary file and then removing it.\n" );
  fprintf( stdout, "\n" );
  fprintf( stdout, "   Algorithm: ( kindly taken from: https://github.com/nollan/zerofile with his permission )\n" );
  fprintf( stdout, "\n" );
  fprintf( stdout, "      The file is filled 4096 bytes at the time ( blocksize ), and halves\n" );
  fprintf( stdout, "      it when disk space is to small for 4096 bytes. So next time it's 2048\n" );
  fprintf( stdout, "      and so on, until it's less than 256 when it will remove the tempfile and\n" );
  fprintf( stdout, "      exit. The reason for halving is to really fill up all the free space with\n" );
  fprintf( stdout, "      zeroes. If the selected block size is larger than the file system block\n" );
  fprintf( stdout, "      size the halving is not really usefull\n" );
  fprintf( stdout, "\n" );
  fprintf( stdout, " Note: CTRL-C kills and cleans up automatically\n" );
  fprintf( stdout, "\n\n" );
  fprintf( stdout, "   %s recognizes the following options:\n\n", program );
  fprintf( stdout, "       -q => No progress report during zeroing, a bit faster\n" );
  fprintf( stdout, "       -b < blkSize > => Amount of zeroes to write at each pass. Default: %u\n", DEFAULT_BLKSIZE );
  fprintf( stdout, "       [ tempFilename ] => Temporary file used. Defaults to: %s\n", DEFAULT_TMP_FILENAME );

}

//****************************************************************************
//
// parseCommandLineOptions
//
//  Description:
//    This procedure takes all the command line options passed to main line.
//
//  Parameters:
//    ( int * )   argc     - The number of command line arguments passed
//                           to this procedure.
//    ( char** )  argv     - List of command line options specified by the user.
//
//  Updates Globals:
//    ( Bool )  quiet       - No progress report during zeroing, a bit faster.
//    ( size_t ) blockSize  - The amount of zeroes to write at each pass.
//    ( char* ) tmpFilename - The Temporary file name to use.
//
//  Returns:
//    ( void )
//*************************************************************************/
void parseCommandLineOptions( int *argc, char *argv[ ] )
{
int opt;
extern char *optarg;
extern int optind;

  // Check for options passed in
  while ( ( opt = getopt( *argc, argv, "hqb:" ) ) != -1 )
  {
     switch ( opt )
     {
        case 'h':
           showHelp( argv[ 0 ] );

           exit( 0 );
        case 'q':
            quiet = TRUE;

            break;
        case 'b':
            blockSize = strtoul( argv[ optind ], NULL, 10 ) ;
            if ( errno == EINVAL )
            {
               fprintf( stderr, "Cannot convert %s to unsigned long. error: %d\n", argv[ optind ], errno );
               exit( errno );
            }

            break;
        case '?':
        default:
            showHelp( argv[ 0 ] );
            fprintf( stderr, "Error: Unknown option given %c.\n", opt );

            exit( 666 );
      }
   }

   // Temp-file selection
   if ( *argc > optind )
      tmpFilename = argv[ optind ];
}

// Cleanup, remove tempfile
void cleanup( )
{
   fprintf( stdout, "Removing temp-file ... " );

   fclose( fid );

   if ( unlink( tmpFilename ) != 0 )
   {
      fprintf( stderr, "error: %d\n", errno );
      exit( errno );
   }
   fprintf( stdout, "Done\n" );
}

void intHandler( )
{
   if ( ! quiet )
      fprintf( stdout, "\n" );

   fprintf( stdout, "Cleaning up" );
   cleanup( );
   exit( 1 );
}

//***********************************************************************
//
// main
//
// Description:
//
// Returns:
//   0 if successfull, otherwise errno.
//***********************************************************************/
int main( int argc, char *argv[ ] )
{
time_t startTime, endTime;
double diff_t;
char* bunchOfZeroes;

   // Keep score of bytes written
   long bytesWritten = 0;
   long totalBytesWritten = 0;

   // Keep score of when started
   time( &startTime );

   // Well, parse the flags! :)
   parseCommandLineOptions( &argc, argv );


   // Handle ctrl-c and kills
   signal( SIGINT, intHandler );
   signal( SIGTERM, intHandler );


   // Make some zeroes
   bunchOfZeroes = ( char * ) calloc( blockSize, sizeof( char * ) );

   fid = fopen( tmpFilename, "w" );
   if ( fid == NULL )
   {
      fprintf( stderr, "error: %d\n", errno );
      exit ( errno );
   }

   fprintf( stdout, "Using tempfile: '%s'\n", tmpFilename );

   // Amount to write at each pass
   size_t amount = blockSize;
   for ( ; ; )
   {
      // Write zeroes up to amount ( which is equal or less than blockSize )
      size_t bytesWritten = fwrite( bunchOfZeroes, 1, amount, fid );
      if ( bytesWritten != amount )
      {
         // Check for no more space left on device
         if ( errno == ENOSPC )
         {
            // if not all of the amount is written, halve it and try again
            // in case of very large blockSize.
            if ( bytesWritten != amount )
               amount /= 2;

            // Limit of amount size, disk buffer size is normally larger
            if ( amount < minAmount )
               break;

         } else
         {
            // Other error
            cleanup( );
            fprintf( stderr, "error: %d\n", errno );
            exit( errno );
         }
      }
      // Update stats
      totalBytesWritten += bytesWritten;

      // Keep quiet if wanted
      if ( ! quiet )
         fprintf( stdout, "Written: %zu bytes        \r", totalBytesWritten );
   }
   // Print some stats
   if ( ! quiet )
      fprintf( stdout, "\n" );

   time( &endTime );
   diff_t = difftime( endTime, startTime );
   fprintf( stdout, "Duration: %f ; Performance: %.3f bytes/sec\n",
      diff_t,
      totalBytesWritten / diff_t );

   cleanup( );

   exit( 0 );
}

