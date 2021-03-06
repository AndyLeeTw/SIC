#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#define MAX_LEN 10
#define MAX_TOKEN 20
#define MAX_ROW 50
#define MAX_TABLE 100
#define FIN -2
#define DO -1
#define UNDO -4
#define NONE -3
#define Ins 0
#define Nins 1
#define Reg 2
#define Del 3
#define Sym 4
#define IR 5
#define Str 6
#define DH 8
#define BYTE 1
#define WORD 3
#define TRUE -1
#define FALSE -2

typedef struct T
{
    int size ;
    char **command ;
} table ;

typedef struct TV
{
    int set ;
    int subset ;
} typevalue ;

typedef struct TL
{
    int loc, row ;
    typevalue cdtv[ MAX_TOKEN ] ;
    char mcode[ MAX_LEN ], code[ MAX_ROW ] ;
    struct TL *next ;
} tvlink ;

typedef struct OPT
{
    char *command ;
    char opc[ 3 ] ;
    int format ;
} optable ;

typedef struct ST
{
    char *sym ;
    int address ;
} symtable ;

typedef struct LT
{
    char *ir_str ;
    int address ;
} litertable ;

typedef struct ET
{
    tvlink *link[ MAX_ROW ] ;
    int size ;
    int done[ MAX_ROW ] ;
} equtable ;

tvlink *createlink( tvlink*, typevalue*, char*, int ) ;
void freelink( tvlink* ) ;
void bulidoptable( optable*, table* ) ;
int checkxe( optable*, tvlink* ) ;
int pass1( optable*, table*, tvlink*, symtable*, litertable* ) ;
int pass2( optable*, table*, tvlink*, symtable*,litertable*, int ) ;
void getTable( char*, table* ) ;
void freetable( table* ) ;
tvlink *bulidtable( char*, table*, tvlink* ) ;
void cuttoken( char*, table*, typevalue*, int* ) ;
void cutss( typevalue*, table*, char*, int* ) ;
void cutsi( typevalue*, table*, char*, int* ) ;
void initializetable( table*, int ) ;
void enarray( typevalue*, int, int, int* ) ;
int hash( char*, table* ) ;
int BWcount( int, int*, symtable*, int, tvlink*, table*, int* ) ;
int format2check( tvlink*, int* ) ;
int format3( table*, symtable*, tvlink*, int, int*, int, int ) ;
int format4( table*, symtable*, tvlink*, int, int* ) ;
int checkendusym( symtable*, int*, char*, int ) ;
void outputfile( tvlink*, char*, table* ) ;
int equ( equtable*, symtable*, table*, int* ) ;

int main()
{
    int i, count = 0, check ;
    int xecheck ;
    char fname[ MAX_ROW ] ;
    tvlink *alink = NULL, *stdlink = NULL ;
    table stdtable[ 7 ] ;
    symtable stdsymtable[ MAX_TABLE ] ;
    litertable stdlitertable[ MAX_TABLE ] ;
    optable stdoptable[ 59 ] ;
    getTable( "Table1.table", &stdtable[ Ins ] ) ;
    getTable( "Table2.table", &stdtable[ Nins ] ) ;
    getTable( "Table3.table", &stdtable[ Reg ] ) ;
    getTable( "Table4.table", &stdtable[ Del ] ) ;
    initializetable( &stdtable[ Sym ], MAX_TABLE ) ;
    initializetable( &stdtable[ IR ], MAX_TABLE ) ;
    initializetable( &stdtable[ Str ], MAX_TABLE ) ;
    bulidoptable( stdoptable, stdtable ) ;
    printf( "Please input filename.\n" ) ;
    scanf( "%s", fname ) ;
    alink = bulidtable( fname, stdtable, stdlink ) ;
    if( alink != NULL )
    {
        xecheck = checkxe( stdoptable, alink ) ;
        check = pass1( stdoptable, stdtable, alink, stdsymtable, stdlitertable ) ;
        if( check == TRUE )
        {
            check = pass2( stdoptable, stdtable, alink, stdsymtable, stdlitertable, xecheck ) ;
            outputfile( alink, fname, stdtable ) ;
        }
        else if( check != FALSE )
            printf("%d row syntax error\n", check ) ;
    }
    for( i = 0 ; i < 7 ; i++ )
        freetable( &stdtable[ i ] ) ;
    freelink( alink ) ;
    return 0 ;
}

void getTable( char *fname, table *atable )
{
    FILE *inTable = NULL ;
    table temp ;
    char temparray[ MAX_LEN ], *temptoken ;
    int tableNo = 0, i, j ;
    if( ( inTable = fopen( fname, "r" ) ) != NULL )
    {
        while( 1 )
        {
            if( feof( inTable ) )
                break ;
            fscanf( inTable, "%s", temparray ) ;
            tableNo++ ;
        }
        if( tableNo != 0 )
        {
            rewind( inTable ) ;
            initializetable( atable, tableNo ) ;
            for( i = 0 ; i < tableNo ; i++ )
            {
                fscanf( inTable, "%s", atable->command[ i ] ) ;
                for( j = 0 ; j < strlen( atable->command[ i ] ) ; j++ )
                   atable->command[ i ][ j ] = toupper( atable->command[ i ][ j ] ) ;
            }
            atable->size = tableNo ;
        }
        fclose( inTable ) ;
    }
}

void initializetable( table *atable, int tableNo )
{
    char *temparray ;
    int i ;
    atable->command = malloc( tableNo * sizeof( char* ) ) ;
    temparray = malloc( tableNo * MAX_LEN * sizeof( char ) ) ;
    for(i = 0; i < tableNo ; i++, temparray += MAX_LEN )
    {
        atable->command[ i ] = temparray ;
        atable->command[ i ][ 0 ] = '\0' ;
    }
}

void freetable( table *atable )
{
    free( atable->command[ 0 ] ) ;
    free( atable->command ) ;
}

tvlink *createlink( tvlink *alink, typevalue *cdtv, char *code, int row )
{
    int i ;
    tvlink *newlink, *linkn ;
    newlink = malloc( sizeof( tvlink ) ) ;
    if( newlink != NULL )
    {
        for( i = 0 ; cdtv[ i ].set != FIN ; i++ )
            newlink->cdtv[ i ] = cdtv[ i ] ;
        newlink->cdtv[ i ].set = FIN ;
        newlink->row = row ;
        strcpy( newlink->code, code ) ;
        newlink->next = NULL ;
        if( alink == NULL )
            alink = newlink ;
        else
        {
            linkn = alink ;
            while( linkn->next != NULL )
            {
                linkn = linkn->next ;
            }
            linkn->next = newlink ;
        }
        return alink ;
    }
    else
    {
        printf( "new a link fail.\n" ) ;
        return NULL ;
    }
}

void freelink( tvlink *alink )
{
    if( alink != NULL )
    {
        if( alink->next != NULL )
            freelink( alink->next ) ;
        free( alink ) ;
    }
}

tvlink *bulidtable( char *fname, table *stdtable, tvlink *alink )
{
    FILE *infile = NULL, *outfile = NULL ;
    char temp[ MAX_ROW ], *token, *loc, code[ MAX_ROW ] ;
    char infname[ MAX_ROW ], outfname[ MAX_ROW ] = "token_" ;
    int i, j, count, row = 1 ;
    strcpy( infname, fname ) ;
    strcat( infname, ".txt" ) ;
    strcat( outfname, infname ) ;
    typevalue cdtv[ MAX_TOKEN ] ;
    if( ( infile = fopen( infname, "r" ) )!= NULL )
    {
        if( ( outfile = fopen( outfname, "w" ) )!= NULL )
        {
            while( 1 )
            {
                count = 0 ;
                cdtv[ 0 ].set = FIN ;
                temp[ 0 ] = '\0' ;
                if( feof( infile ) )
                    break ;
                fgets( temp, MAX_ROW, infile ) ;
                if( temp[ strlen( temp ) - 1 ] == '\n' )
                    temp[ strlen( temp ) - 1 ] = '\0' ;
                strcpy( code, temp ) ;
                fprintf( outfile, "%s\n", temp ) ;
                token = strtok( temp, " \t" ) ;
                while( token != NULL )
                {
                    if( ( loc = strstr( token, "." ) ) != NULL )
                    {
                        token[ loc - token ] = '\0' ;
                    }
                    i = j = 0 ;
                    while( token[ i ] != '\0' )
                    {
                        if( token[ i ] == '\'' )
                            j++ ;
                        if( j < 1 || j > 2 )
                        {
                            if( token[ i ] >= 'a' && token[ i ] <= 'z' )
                            token[ i ] -= 32 ;
                        }
                        i++;
                        if( j == 2 )
                            j %= 2 ;
                    }
                    cuttoken( token, stdtable, cdtv, &count ) ;
                    if( loc != NULL )
                    {
                        cdtv[ 0 ].set = FIN ;
                        break ;
                    }
                    token = strtok( NULL, " \t" ) ;
                }
                alink = createlink( alink, cdtv, code, row ) ;
                if( alink == NULL )
                    return alink ;
                row++ ;
                for( i = 0 ; cdtv[ i ].set != FIN ; i++ )
                    fprintf( outfile, "(%d,%d)", cdtv[ i ].set + 1, cdtv[ i ].subset + 1 ) ;
                fprintf( outfile, "\n" ) ;

            }
            fclose( outfile ) ;
        }
        fclose( infile ) ;
    }
    else
        printf( "%s is not exist.\n", infname ) ;
    return alink ;
}

void outputfile( tvlink *alink, char *fname, table *atable )
{
    FILE *outfile ;
    char outfname[ MAX_ROW ] = "output_" ;
    strcat( outfname, fname ) ;
    strcat( outfname, ".txt" ) ;
    if( ( outfile = fopen( outfname, "w" ) )!= NULL )
    {
        fprintf( outfile, "Line\tLoc\tSource statement\t\tObject code\n" ) ;
        while( alink != NULL )
        {

            if( alink->cdtv[ 0 ].set != FIN )
            {
                if( strcmp( atable[ alink->cdtv[ 0 ].set ].command[ alink->cdtv[ 0 ].subset ], "LTORG" ) == 0 || strcmp( atable[ alink->cdtv[ 0 ].set ].command[ alink->cdtv[ 0 ].subset ], "BASE" ) == 0 )
                    fprintf( outfile, "%03d\t\t%-20s\n", alink->row, alink->code ) ;
                else if( alink->cdtv[ 0 ].set == Nins && alink->cdtv[ 0 ].subset == 1 )
                    fprintf( outfile, "%03d\t\t%-20s\t\t%s\n", alink->row, alink->code, alink->mcode ) ;
                else
                    fprintf( outfile, "%03d\t%04x\t%-20s\t\t%s\n", alink->row, alink->loc, alink->code, alink->mcode ) ;
            }
            else
                fprintf( outfile, "%03d\t%-20s\n", alink->row, alink->code ) ;
            if( alink->cdtv[ 0 ].set != FIN )
            {
                if( alink->cdtv[ 0 ].set == Nins && alink->cdtv[ 0 ].subset == 1  )
                    printf( "%03d\t\t%-20s\n", alink->row, alink->code ) ;
                else if( strcmp( atable[ alink->cdtv[ 0 ].set ].command[ alink->cdtv[ 0 ].subset ], "LTORG" ) == 0 || strcmp( atable[ alink->cdtv[ 0 ].set ].command[ alink->cdtv[ 0 ].subset ], "BASE" ) == 0 )
                    printf( "%03d\t\t%-20s\n", alink->row, alink->code ) ;
                else
                printf( "%03d\t%04x\t%-20s\t\t%s\n", alink->row, alink->loc, alink->code, alink->mcode ) ;
            }
            else
                printf( "%03d\t%-20s\n", alink->row, alink->code ) ;
            if( alink->next != NULL )
                alink = alink->next ;
            else
                break ;
        }
        fclose( outfile ) ;
    }
}

void cuttoken( char *token, table *stdtable, typevalue *cdtv, int *count )
{
    int i, j, done ;
    int minv ;
    char tokenD[ MAX_TOKEN ], *loc ;
    char *min = NULL ;
    done = UNDO ;
    for( i = 0 ; i < 4 ; i++ )
    {
        for( j = 0 ; j < stdtable[ i ].size ; j++ )
        {
            if( strcmp( token, stdtable[ i ].command[ j ] ) == 0 )
            {
                enarray( cdtv, i, j, count ) ;
                done = DO ;
            }
        }
        if( done == DO )
            break ;
    }
    if( done == UNDO )
    {
        for( i = 0 ; i < stdtable[ Del ].size ; i++ )
        {
            loc = strstr( token, stdtable[ Del ].command[ i ] ) ;
            if( loc != NULL )
            {
                if( min == NULL || loc < min )
                {
                    min = loc ;
                    minv = i ;
                }

            }
        }
        if( min != NULL )
        {
            if( cdtv[ *count ].set == NONE )
                ( *count )-- ;
            if( min - token == 0 )
            {
                strcpy( tokenD, token + 1 ) ;
                enarray( cdtv, Del, minv, count ) ;
                cuttoken( tokenD, stdtable, cdtv, count ) ;
            }
            else if( min - token == strlen( token ) - 1 )
            {
                strcpy( tokenD, token ) ;
                tokenD[ strlen( tokenD ) - 1 ] = '\0' ;
                cuttoken( tokenD, stdtable, cdtv, count ) ;
                enarray( cdtv, Del, minv, count ) ;
            }
            else
            {
                if( minv == DH && cdtv[ *count ].set != NONE )
                    cutss( cdtv, stdtable, token, count ) ;
                else
                {
                    strcpy( tokenD, token ) ;
                    tokenD[ min - token ] = '\0' ;
                    cuttoken( tokenD, stdtable, cdtv, count ) ;
                    enarray( cdtv, Del, minv, count ) ;
                    strcpy( tokenD, token + ( min - token + 1 ) ) ;
                    cuttoken( tokenD, stdtable, cdtv, count ) ;
                }
            }
        }
        else
            cutsi( cdtv, stdtable, token, count ) ;
    }
    cdtv[ *count ].set = FIN ;
}

void cutss( typevalue *cdtv, table *stdtable, char *token, int *count )
{
    char *loc, tokenD[ MAX_TOKEN ] ;
    int i, in ;
    loc = strstr( token, "'" ) ;
    if( loc != NULL )
    {
        if( loc - token == 1 )
        {
            if( token[ 0 ] == 'C' )
                in = Str ;
            if( token[ 0 ] == 'X' )
                in = IR ;
            enarray( cdtv, Del, DH, count ) ;
            for( i = loc - token + 1 ; i < strlen( token ) ; i++ )
                tokenD[ i - ( loc - token + 1 ) ] = token[ i ] ;
            tokenD[ i - ( loc - token + 1 ) ] = '\0' ;
            loc = strstr( tokenD, "'" ) ;
            if( loc != NULL )
            {
                if( loc - tokenD == 0 )
                    enarray( cdtv, Del, DH, count ) ;
                if( loc - tokenD == strlen( tokenD ) - 1 )
                {
                    tokenD[ loc - tokenD ] = '\0' ;
                    enarray( cdtv, in, hash( tokenD, &stdtable[ in ] ), count ) ;
                    enarray( cdtv, Del, DH, count ) ;
                }
                else
                {
                    tokenD[ loc - tokenD ] = '\0' ;
                    enarray( cdtv, in, hash( tokenD, &stdtable[ in ] ), count ) ;
                    for( i = loc - tokenD + 1 ; i < strlen( tokenD ) ; i++ )
                        tokenD[ i - ( loc - tokenD + 1 ) ] = tokenD[ i ] ;
                    tokenD[ i - ( loc - tokenD + 1 ) ] = '\0' ;
                    cuttoken( tokenD, stdtable, cdtv, count ) ;
                }
            }
        }
    }
    else
    {
        enarray( cdtv, NONE, NONE, count ) ;
        cuttoken( token, stdtable, cdtv, count ) ;
    }
}

void cutsi( typevalue *cdtv, table *stdtable, char *token, int *count )
{
    int i ;
    if( strlen( token ) < 5 )
    {
        for( i = 0 ; i < strlen( token ) ; i++ )
        {
            if( ( token[ i ] < '0' || token[ i ] > '9' ) && ( token[ i ] < 'A' || token[ i ] > 'F' ) )
            {
                i = NONE ;
                break ;
            }
        }
        if( i == NONE )
        {
            enarray( cdtv, Sym, hash( token, &stdtable[ Sym ] ), count ) ;
        }
        else
            enarray( cdtv, IR, hash( token, &stdtable[ IR ] ), count ) ;
    }
    else
        enarray( cdtv, Sym, hash( token, &stdtable[ Sym ] ), count ) ;
}

int hash( char *token, table *atable )
{
    int i, hashvalue = 0 ;
    for( i = 0 ; i < strlen( token ) ; i++ )
        hashvalue += token[ i ] ;
    hashvalue %= MAX_TABLE ;
    if( strcmp( token, atable->command[ hashvalue ] ) != 0 )
    {
        while( atable->command[ hashvalue ][ 0 ] != '\0' && strcmp( token, atable->command[ hashvalue ] ) != 0 )
        {
            hashvalue++ ;
            if( hashvalue >= MAX_TABLE )
                hashvalue %= MAX_TABLE ;
        }
        strcpy( atable->command[ hashvalue ], token ) ;
        return hashvalue ;
    }
    else
        return hashvalue ;
}

void enarray( typevalue *cdtv, int set, int subset, int *loc )
{
    cdtv[ *loc ].set = set ;
    cdtv[ *loc ].subset = subset ;
    ( *loc )++ ;
}

void bulidoptable( optable *aoptable, table *atable )
{
    int i ;
    for( i = 0 ; i < atable[ Ins ].size ; i++ )
        aoptable[ i ].command = atable[ Ins ].command[ i ] ;
    aoptable[ 0 ].format = 3 ; strcpy( aoptable[ 0 ].opc, "18" ) ;
    aoptable[ 1 ].format = 3 ; strcpy( aoptable[ 1 ].opc, "58" ) ;
    aoptable[ 2 ].format = 2 ; strcpy( aoptable[ 2 ].opc, "90" ) ;
    aoptable[ 3 ].format = 3 ; strcpy( aoptable[ 3 ].opc, "40" ) ;
    aoptable[ 4 ].format = 2 ; strcpy( aoptable[ 4 ].opc, "B4" ) ;
    aoptable[ 5 ].format = 3 ; strcpy( aoptable[ 5 ].opc, "28" ) ;
    aoptable[ 6 ].format = 3 ; strcpy( aoptable[ 6 ].opc, "88" ) ;
    aoptable[ 7 ].format = 2 ; strcpy( aoptable[ 7 ].opc, "A0" ) ;
    aoptable[ 8 ].format = 3 ; strcpy( aoptable[ 8 ].opc, "24" ) ;
    aoptable[ 9 ].format = 3 ; strcpy( aoptable[ 9 ].opc, "64" ) ;
    aoptable[ 10 ].format = 2 ; strcpy( aoptable[ 10 ].opc, "9C" ) ;
    aoptable[ 11 ].format = 1 ; strcpy( aoptable[ 11 ].opc, "C4" ) ;
    aoptable[ 12 ].format = 1 ; strcpy( aoptable[ 12 ].opc, "C0" ) ;
    aoptable[ 13 ].format = 1 ; strcpy( aoptable[ 13 ].opc, "F4" ) ;
    aoptable[ 14 ].format = 3 ; strcpy( aoptable[ 14 ].opc, "3C" ) ;
    aoptable[ 15 ].format = 3 ; strcpy( aoptable[ 15 ].opc, "30" ) ;
    aoptable[ 16 ].format = 3 ; strcpy( aoptable[ 16 ].opc, "34" ) ;
    aoptable[ 17 ].format = 3 ; strcpy( aoptable[ 17 ].opc, "38" ) ;
    aoptable[ 18 ].format = 3 ; strcpy( aoptable[ 18 ].opc, "48" ) ;
    aoptable[ 19 ].format = 3 ; strcpy( aoptable[ 19 ].opc, "00" ) ;
    aoptable[ 20 ].format = 3 ; strcpy( aoptable[ 20 ].opc, "68" ) ;
    aoptable[ 21 ].format = 3 ; strcpy( aoptable[ 21 ].opc, "50" ) ;
    aoptable[ 22 ].format = 3 ; strcpy( aoptable[ 22 ].opc, "70" ) ;
    aoptable[ 23 ].format = 3 ; strcpy( aoptable[ 23 ].opc, "08" ) ;
    aoptable[ 24 ].format = 3 ; strcpy( aoptable[ 24 ].opc, "6C" ) ;
    aoptable[ 25 ].format = 3 ; strcpy( aoptable[ 25 ].opc, "74" ) ;
    aoptable[ 26 ].format = 3 ; strcpy( aoptable[ 26 ].opc, "04" ) ;
    aoptable[ 27 ].format = 3 ; strcpy( aoptable[ 27 ].opc, "D0" ) ;
    aoptable[ 28 ].format = 3 ; strcpy( aoptable[ 28 ].opc, "20" ) ;
    aoptable[ 29 ].format = 3 ; strcpy( aoptable[ 29 ].opc, "60" ) ;
    aoptable[ 30 ].format = 2 ; strcpy( aoptable[ 30 ].opc, "98" ) ;
    aoptable[ 31 ].format = 1 ; strcpy( aoptable[ 31 ].opc, "C8" ) ;
    aoptable[ 32 ].format = 3 ; strcpy( aoptable[ 32 ].opc, "44" ) ;
    aoptable[ 33 ].format = 3 ; strcpy( aoptable[ 33 ].opc, "D8" ) ;
    aoptable[ 34 ].format = 2 ; strcpy( aoptable[ 34 ].opc, "AC" ) ;
    aoptable[ 35 ].format = 3 ; strcpy( aoptable[ 35 ].opc, "4C" ) ;
    aoptable[ 36 ].format = 2 ; strcpy( aoptable[ 36 ].opc, "A4" ) ;
    aoptable[ 37 ].format = 2 ; strcpy( aoptable[ 37 ].opc, "A8" ) ;
    aoptable[ 38 ].format = 1 ; strcpy( aoptable[ 38 ].opc, "F0" ) ;
    aoptable[ 39 ].format = 3 ; strcpy( aoptable[ 39 ].opc, "EC" ) ;
    aoptable[ 40 ].format = 3 ; strcpy( aoptable[ 40 ].opc, "0C" ) ;
    aoptable[ 41 ].format = 3 ; strcpy( aoptable[ 41 ].opc, "78" ) ;
    aoptable[ 42 ].format = 3 ; strcpy( aoptable[ 42 ].opc, "54" ) ;
    aoptable[ 43 ].format = 3 ; strcpy( aoptable[ 43 ].opc, "80" ) ;
    aoptable[ 44 ].format = 3 ; strcpy( aoptable[ 44 ].opc, "D4" ) ;
    aoptable[ 45 ].format = 3 ; strcpy( aoptable[ 45 ].opc, "14" ) ;
    aoptable[ 46 ].format = 3 ; strcpy( aoptable[ 46 ].opc, "7C" ) ;
    aoptable[ 47 ].format = 3 ; strcpy( aoptable[ 47 ].opc, "E8" ) ;
    aoptable[ 48 ].format = 3 ; strcpy( aoptable[ 48 ].opc, "84" ) ;
    aoptable[ 49 ].format = 3 ; strcpy( aoptable[ 49 ].opc, "10" ) ;
    aoptable[ 50 ].format = 3 ; strcpy( aoptable[ 50 ].opc, "1C" ) ;
    aoptable[ 51 ].format = 3 ; strcpy( aoptable[ 51 ].opc, "5C" ) ;
    aoptable[ 52 ].format = 2 ; strcpy( aoptable[ 52 ].opc, "94" ) ;
    aoptable[ 53 ].format = 2 ; strcpy( aoptable[ 53 ].opc, "B0" ) ;
    aoptable[ 54 ].format = 3 ; strcpy( aoptable[ 54 ].opc, "E0" ) ;
    aoptable[ 55 ].format = 1 ; strcpy( aoptable[ 55 ].opc, "F8" ) ;
    aoptable[ 56 ].format = 3 ; strcpy( aoptable[ 56 ].opc, "2C" ) ;
    aoptable[ 57 ].format = 2 ; strcpy( aoptable[ 57 ].opc, "B8" ) ;
    aoptable[ 58 ].format = 3 ; strcpy( aoptable[ 58 ].opc, "DC" ) ;
}

int checkxe( optable *aoptable, tvlink *alink )
{
    int xe[ 16 ] = { 1, 6, 9, 20, 22, 24, 25, 27, 29, 39, 41, 43, 44, 46, 48, 51 } ;
    int i, j ;
    while( alink != NULL )
    {
        for( i = 0 ; alink->cdtv[ i ].set != FIN ; i++ )
        {
            if( alink->cdtv[ i ].set == Ins )
            {
                if( alink->cdtv[ i - 1 ].set == Del && alink->cdtv[ i - 1 ].subset == 1 )
                {
                    if( aoptable[ alink->cdtv[ i ].subset ].format == 3 )
                        return TRUE ;
                    else
                    {
                        printf( "%d row syntax error.\n", alink->row ) ;
                        return FIN ;
                    }
                }
                if( aoptable[ alink->cdtv[ i ].subset ].format == 1 || aoptable[ alink->cdtv[ i ].subset ].format == 2 )
                    return TRUE ;
                for( j = 0 ; j < 16 ; j++ )
                {
                    if( alink->cdtv[ i ].subset == xe[ j ] )
                        return TRUE ;
                }
            }
        }
        if( alink->next != NULL )
            alink = alink->next ;
        else
            break ;
    }
    return FALSE ;
}

int pass1( optable *aoptable, table *stdtable, tvlink *alink, symtable *asymtable, litertable *alitertable )
{
    int i, loc = 0, symc = 0, litc = 0, litcur = 0 ;
    int x, j, k, check, buf ;
    char buffer[ MAX_LEN ] ;
    equtable aequtable ;
    aequtable.size = 0 ;
    while( alink != NULL )
    {
        for( i = 0 ; alink->cdtv[ i ].set != FIN ; i++ )
        {
            if( alink->cdtv[ i ].set == Nins )
            {
                i++ ;
                if( alink->cdtv[ i - 1 ].subset == 0 )
                {
                    if( alink->cdtv[ i ].set  == IR )
                    {
                        sscanf( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], "%x", &alink->loc ) ;
                        loc = alink->loc ;
                        if( i >= 2 )
                        {
                            if( alink->cdtv[ i - 2 ].set == Sym || alink->cdtv[ i - 2 ].set == IR )
                            {
                                if( alink->cdtv[ i - 2 ].set == IR )
                                {
                                    alink->cdtv[ i - 2 ].set = Sym ;
                                    alink->cdtv[ i - 2 ].subset = hash( stdtable[ IR ].command[ alink->cdtv[ i - 2 ].subset ], &stdtable[ Sym ] ) ;
                                }
                                if( checkendusym( asymtable, &symc, stdtable[ alink->cdtv[ i - 2 ].set ].command[ alink->cdtv[ i - 2 ].subset ], loc ) == FALSE )
                                    return FALSE ;
                            }
                        }
                    }
                    else
                        return alink->row ;
                }
                else if( alink->cdtv[ i - 1 ].subset == 1 )
                {
                    if( alink->cdtv[ i ].set == Sym )
                    {
                        alink->loc = loc ;
                        for( j = litcur ; j < litc ; j++ )
                        {
                            alitertable[ j ].address = loc ;
                            if( checkendusym( asymtable, &symc, alitertable[ j ].ir_str, loc ) == FALSE )
                                return FALSE ;
                            loc += WORD ;
                        }
                    }
                    else
                        return alink->row ;
                }
                else if( alink->cdtv[ i - 1 ].subset == 2 )
                {
                    x = 1 ;
                    i++ ;
                    if( alink->cdtv[ i ].set == Str )
                    {
                        x = strlen( stdtable[ Str ].command[ alink->cdtv[ i ].subset ] ) ;
                        strcpy( alink->mcode, "" ) ;
                        for( j = 0 ; j < strlen( stdtable[ Str ].command[ alink->cdtv[ i ].subset ] ) ; j++ )
                        {
                            sprintf( buffer, "%x", stdtable[ Str ].command[ alink->cdtv[ i ].subset ][ j ] ) ;
                            strcat( alink->mcode, buffer ) ;
                        }
                        if( BWcount( i - 1, &loc, asymtable, BYTE * x, alink, stdtable, &symc ) == FALSE )
                            return FALSE ;
                        i++ ;
                    }
                    else if( alink->cdtv[ i ].set == IR )
                    {
                        BWcount( i - 1, &loc, asymtable, BYTE, alink, stdtable, &symc ) ;
                        strcpy( alink->mcode, stdtable[ IR ].command[ alink->cdtv[ i ].subset ] ) ;
                        if( BWcount( i - 1, &loc, asymtable, BYTE * x, alink, stdtable, &symc ) == FALSE )
                            return FALSE ;
                        i++ ;
                    }
                    else
                        return alink->row ;
                }
                else if( alink->cdtv[ i - 1 ].subset == 3 )
                {
                    if( alink->cdtv[ i ].set == Str || alink->cdtv[ i ].set == IR )
                    {
                        if( BWcount( i, &loc, asymtable, WORD, alink, stdtable, &symc ) == FALSE )
                            return FALSE ;
                        strcpy( alink->mcode, "" ) ;
                        sscanf( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], "%d", &buf ) ;
                        sprintf( buffer, "%x", buf ) ;
                        for( j = 0 ; j < 6 - strlen( buffer ) ; j++ )
                            strcat( alink->mcode, "0" ) ;
                        strcat( alink->mcode, buffer ) ;
                    }
                    else
                        return alink->row ;
                }
                else if( alink->cdtv[ i - 1 ].subset == 4 )
                {
                    x = 1 ;
                    if( alink->cdtv[ i ].set  == IR )
                    {
                        sscanf( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], "%d", &x ) ;
                        if( BWcount( i, &loc, asymtable, BYTE * x, alink, stdtable, &symc ) == FALSE )
                           return FALSE ;
                    }
                    else
                        return alink->row ;
                }
                else if( alink->cdtv[ i - 1 ].subset == 5 )
                {
                    x = 1 ;
                    if( alink->cdtv[ i ].set  == IR )
                    {
                        sscanf( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], "%d", &x ) ;
                        if( BWcount( i, &loc, asymtable, WORD * x, alink, stdtable, &symc ) == FALSE )
                            return FALSE ;
                    }
                    else
                        return alink->row ;
                }
                else
                    return alink->row ;
                if( alink->cdtv[ i + 1 ].set == FIN )
                    break ;
                else
                    return alink->row ;
            }
            else if( alink->cdtv[ i ].set == Ins )
            {
                alink->loc = loc ;
                loc += aoptable[ alink->cdtv[ i ].subset ].format ;
                i++ ;
                if( alink->cdtv[ i - 2 ].set == Del && alink->cdtv[ i - 2 ].subset == 1 )
                    loc++ ;
                if( alink->cdtv[ i - 2 ].set == Sym || alink->cdtv[ i - 2 ].set == IR && i >= 2 )
                {
                    if( alink->cdtv[ i - 2 ].set == IR )
                    {
                        alink->cdtv[ i - 2 ].set = Sym ;
                        alink->cdtv[ i - 2 ].subset = hash( stdtable[ IR ].command[ alink->cdtv[ i - 2 ].subset ], &stdtable[ Sym ] ) ;
                    }
                    if( checkendusym( asymtable, &symc, stdtable[ Sym ].command[ alink->cdtv[ i - 2 ].subset ], loc  ) == FALSE )
                        return FALSE ;
                }
                else if( alink->cdtv[ i - 2 ].set == Del && alink->cdtv[ i - 2 ].subset == 1 && i >= 3 )
                {
                    if( alink->cdtv[ i - 3 ].set == Sym || alink->cdtv[ i - 3 ].set == IR )
                    {
                        if( alink->cdtv[ i - 3 ].set == IR )
                        {
                            alink->cdtv[ i - 3 ].set = Sym ;
                            alink->cdtv[ i - 3 ].subset = hash( stdtable[ IR ].command[ alink->cdtv[ i - 2 ].subset ], &stdtable[ Sym ] ) ;
                        }
                        if( checkendusym( asymtable, &symc, stdtable[ Sym ].command[ alink->cdtv[ i - 3 ].subset ], loc  ) == FALSE )
                            return FALSE ;
                    }
                }
                if( aoptable[ alink->cdtv[ i - 1 ].subset ].format == 1 || alink->cdtv[ i - 1 ].subset == 35 )
                {
                    if( alink->cdtv[ i ].set == FIN )
                        i-- ;
                    else
                        return alink->row ;
                }
                else if( alink->cdtv[ i ].set == Reg && aoptable[ alink->cdtv[ i - 1 ].subset ].format == 2 )
                {
                    check = format2check( alink, &i ) ;
                    if( check == FALSE )
                        return alink->row ;
                }
                else if( alink->cdtv[ i ].set == Sym || alink->cdtv[ i ].set == IR )
                {
                    if( alink->cdtv[ i ].set == IR )
                    {
                        for( j = 0 ; j < symc ; j++ )
                        {
                            if( strcmp( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], asymtable[ j ].sym ) == 0 )
                            {
                                alink->cdtv[ i ].set = Sym ;
                                alink->cdtv[ i ].subset = hash( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], &stdtable[ Sym ] ) ;
                                break ;
                            }
                        }
                    }
                    if( alink->cdtv[ i ].set != Sym )
                        return alink->row ;
                    else
                    {
                        i++ ;
                        if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 0 )
                        {
                            i++ ;
                            if( alink->cdtv[ i ].set != Reg || alink->cdtv[ i ].subset != 1 )
                                return alink->row ;
                        }
                        else if( alink->cdtv[ i ].set == FIN )
                            i-- ;
                        else
                            return alink->row ;
                    }
                }
                else if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset >= 10 )
                {
                    if( alink->cdtv[ i ].subset == 10 )
                    {
                        i++ ;
                        if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == DH )
                        {
                            i++ ;
                            if( alink->cdtv[ i ].set == Str || alink->cdtv[ i ].set == IR )
                            {
                                for( j = 0 ; j < litc ; j++ )
                                    if( alitertable[ j ].ir_str == stdtable[ alink->cdtv[ i ].set ].command[ alink->cdtv[ i ].subset ] )
                                        break ;
                                if( j == litc )
                                {
                                    alitertable[ litc ].ir_str = stdtable[ alink->cdtv[ i ].set ].command[ alink->cdtv[ i ].subset ] ;
                                    litc++ ;
                                }
                                i++ ;
                            }
                        }
                    }
                    else if( alink->cdtv[ i ].subset == 11 || alink->cdtv[ i ].subset == 12 )
                    {
                        i++ ;
                        if( alink->cdtv[ i ].set == IR )
                        {
                            for( j = 0 ; j < symc ; j++ )
                            {
                                if( strcmp( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], asymtable[ j ].sym ) == 0 )
                                {
                                    alink->cdtv[ i ].set = Sym ;
                                    alink->cdtv[ i ].subset = hash( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], &stdtable[ Sym ] ) ;
                                    break ;
                                }
                            }
                        }
                        if( alink->cdtv[ i - 1 ].subset == 12 )
                            if( alink->cdtv[ i ].set != Sym )
                                return alink->row ;
                    }
                }
                else
                    return alink->row ;
                if( alink->cdtv[ i + 1 ].set == FIN )
                    break ;
                else
                {
                    printf( "%d %s", i, alink->code );
                    return alink->row ;
                }
            }
            else if( alink->cdtv[ i ].set == Sym )
            {
                if( strcmp( stdtable[ Sym ].command[ alink->cdtv[ i ].subset ], "EQU" ) == 0 )
                {
                    i++ ;
                    if( alink->cdtv[ i - 2 ].set == Sym && i >= 2 )
                    {
                        if( alink->cdtv[ i ].set == IR )
                        {
                            for( j = 0 ; j < symc ; j++ )
                            {
                                if( strcmp( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], asymtable[ j ].sym ) == 0 )
                                {
                                    alink->cdtv[ i ].set = Sym ;
                                    alink->cdtv[ i ].subset = hash( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], &stdtable[ Sym ] ) ;
                                    break ;
                                }
                            }
                            if( alink->cdtv[ i ].set == IR )
                            {
                                asymtable[ symc ].sym = stdtable[ Sym ].command[ alink->cdtv[ i - 2 ].subset ] ;
                                sscanf( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], "%d", &asymtable[ symc ].address ) ;
                                alink->loc = asymtable[ symc ].address ;
                                symc++ ;
                                i++ ;
                            }
                        }
                        if( alink->cdtv[ i ].set == Sym )
                        {
                            i++ ;
                            for( ; alink->cdtv[ i ].set != FIN ; i++ )
                            {
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset >= 1 && alink->cdtv[ i ].subset <= 4 )
                                {
                                    i++ ;
                                    if( alink->cdtv[ i ].set != Sym )
                                        return alink->row ;
                                }
                                else
                                {
                                    return alink->row ;
                                }
                            }
                            aequtable.done[ aequtable.size ] = UNDO ;
                            aequtable.link[ aequtable.size ] = alink ;
                            aequtable.size++ ;
                        }
                        if( alink->cdtv[ i ].set == FIN )
                            i-- ;
                        else
                        {
                            return alink->row ;
                        }
                    }
                    else
                        return alink->row ;
                    for( j = 0 ; j < symc - 1 ; j++ )
                    {
                        if( asymtable[ j ].sym == asymtable[ symc - 1 ].sym )
                        {
                            printf( "%d row duplicate symbol", alink->row ) ;
                            return FALSE ;
                        }
                    }
                }
                else if( strcmp( stdtable[ Sym ].command[ alink->cdtv[ i ].subset ], "BASE" ) == 0 )
                {
                    i++ ;
                    if( alink->cdtv[ i - 2 ].set == Sym && i >= 2 )
                        return FALSE ;
                    if( alink->cdtv[ i ].set != Sym )
                        return alink->row ;
                }
                else if( strcmp( stdtable[ Sym ].command[ alink->cdtv[ i ].subset ], "LTORG" ) == 0 )
                {
                    for( j = litcur ; j < litc ; j++ )
                    {
                        alitertable[ j ].address = loc ;
                        asymtable[ symc ].sym = alitertable[ j ].ir_str ;
                        asymtable[ symc ].address = loc ;
                        symc++ ;
                        loc += WORD ;
                    }
                    litcur = litc ;
                }
            }
        }
        if( alink->next != NULL )
            alink = alink->next ;
        else
            break ;
    }
    equ( &aequtable, asymtable, stdtable, &symc ) ;
    asymtable[ symc ].address = FIN ;
    alitertable[ litc ].address = FIN ;
    return TRUE ;
}

int equ( equtable *aequtable , symtable *asymtable, table *atable, int *symc )
{
    int i, ctc, k, done = 0, checkdo, address ;
    while( done != aequtable->size )
    {
        for( i = 0 ; i < aequtable->size ; i++ )
        {
            address = UNDO ;
            if( aequtable->done[ i ] == UNDO )
            {
                for( ctc = 2 ; aequtable->link[ i ]->cdtv[ ctc ].set != FIN ; ctc++ )
                {
                    if( ctc == 2 && aequtable->link[ i ]->cdtv[ ctc + 1 ].set == FIN )
                    {
                        for( k = 0 ; k < *symc ; k++ )
                        {
                            if( atable[ Sym ].command[ aequtable->link[ i ]->cdtv[ ctc ].subset ] == asymtable[ k ].sym )
                            {
                                checkendusym( asymtable, symc, atable[ Sym ].command[ aequtable->link[ i ]->cdtv[ ctc - 2 ].subset ], asymtable[ k ].address ) ;
                                printf( "%x\n",asymtable[ k ].address ) ;
                                aequtable->link[ i ]->loc = asymtable[ k ].address ;
                                aequtable->done[ i ] = DO ;
                                break ;
                            }
                        }
                        break ;
                    }
                    else if( ctc == 2 )
                    {
                        for( k = 0 ; k < *symc ; k++ )
                        {
                            if( atable[ Sym ].command[ aequtable->link[ i ]->cdtv[ ctc ].subset ] == asymtable[ k ].sym )
                            {
                                address = asymtable[ k ].address ;
                                break ;
                            }
                        }
                        if( address == UNDO )
                            break ;
                    }
                    else
                    {
                        ctc++ ;
                        for( k = 0 ; k < *symc ; k++ )
                        {
                            if( atable[ Sym ].command[ aequtable->link[ i ]->cdtv[ ctc ].subset ] == asymtable[ k ].sym )
                                break ;
                        }
                        if( address == UNDO )
                            break ;
                        switch( aequtable->link[ i ]->cdtv[ ctc - 1 ].subset )
                        {
                            case 1:
                                address += asymtable[ k ].address ;
                                break ;
                            case 2:
                                address -= asymtable[ k ].address ;
                                break ;
                            case 3:
                                address *= asymtable[ k ].address ;
                                break ;
                            case 4:
                                address /= asymtable[ k ].address ;
                                break ;
                        }
                    }
                    if( aequtable->link[ i ]->cdtv[ ctc + 1 ].set == FIN )
                    {
                        checkendusym( asymtable, symc, atable[ Sym ].command[ aequtable->link[ i ]->cdtv[ 0 ].subset ], address ) ;
                        aequtable->link[ i ]->loc = address ;
                        aequtable->done[ i ] = DO ;
                        break ;
                    }
                }
            }
        }
        for( i = checkdo =  0 ; i < aequtable->size ; i++ )
        {
            if( aequtable->done[ i ] == DO )
            {
                checkdo++ ;
            }
        }
        printf("%d",checkdo ) ;
        if( checkdo <= done )
        {
            printf( "EQU fail.\n" ) ;
            return FALSE ;
        }
        else
            done = checkdo ;
    }
}

int BWcount( int count, int *loc, symtable *asymtable, int BW, tvlink *alink, table *stdtable, int *symc )
{
    int i ;
    alink->loc = *loc ;
    if( count >= 2 )
    {
        if( alink->cdtv[ count - 2 ].set == IR )
        {
            alink->cdtv[ count - 2 ].set = Sym ;
            alink->cdtv[ count - 2 ].subset = hash( stdtable[ IR ].command[ alink->cdtv[ count - 2 ].subset ], &stdtable[ Sym ] ) ;
        }
        if( alink->cdtv[ count - 2 ].set == Sym )
            if( checkendusym( asymtable, symc, stdtable[ Sym ].command[ alink->cdtv[ count - 2 ].subset ], *loc ) == FALSE )
            {
                printf( "%d row duplicate symbol.\n", alink->row ) ;
                return FALSE ;
            }
    }
    *loc += BW ;
    return TRUE ;
}

int checkendusym( symtable *asymtable, int *symc, char *sym, int loc )
{
    int i ;
    for( i = 0 ; i < *symc ; i++ )
    {
        if( asymtable[ i ].sym == asymtable[ *symc ].sym )
            return FALSE ;
    }
    asymtable[ *symc ].sym = sym ;
    asymtable[ *symc ].address = loc ;
    ( *symc )++ ;
    return TRUE ;
}

int format2check( tvlink *alink, int *loc )
{
    if( alink->cdtv[ *loc - 1 ].subset == 4 || alink->cdtv[ *loc - 1 ].subset == 53 || alink->cdtv[ *loc - 1 ].subset == 57 )
    {
        ( *loc )++ ;
        if( alink->cdtv[ *loc ].set == FIN )
        {
            ( *loc )-- ;
            return TRUE ;
        }
        else
            return FALSE ;
    }
    else
    {
        if( alink->cdtv[ *loc ].set == Reg )
        {
            ( *loc )++ ;
            if( alink->cdtv[ *loc ].set == Del && alink->cdtv[ *loc ].subset == 0 )
            {
                ( *loc )++ ;
                if( alink->cdtv[ *loc ].set == Reg )
                {
                    ( *loc )++ ;
                    if( alink->cdtv[ *loc ].set == FIN )
                    {
                        ( *loc )-- ;
                        return TRUE ;
                    }
                    else
                        return FALSE ;
                }
                else
                    return FALSE ;
            }
            else
                return FALSE ;
        }
        else
            return FALSE ;
    }
}

int pass2( optable *aoptable, table *atable, tvlink *alink, symtable *asymtable, litertable *alitertable, int check )
{
    int i, j, opc ;
    int pc = 0 , direct, address, breg = FIN ;
    int index = 8, base = 4, prog = 2, format = 1, flag ;
    int disp ;
    char reg_adr[ MAX_LEN ], sflag[ MAX_LEN ] ;
    if( alink != NULL )
        pc = alink->loc ;
    while( alink != NULL )
    {
        if( alink->next != NULL )
            pc = alink->next->loc ;
        else
            pc = FIN ;
        for( i = 0 ; alink->cdtv[ i ].set != FIN ; i++ )
        {
            if( alink->cdtv[ i ].set == Ins )
            {
                if( aoptable[ alink->cdtv[ i ].subset ].format == 1 )
                    strcpy( alink->mcode, aoptable[ alink->cdtv[ i ].subset ].opc ) ;
                else if( aoptable[ alink->cdtv[ i ].subset ].format == 2 )
                {
                    strcpy( alink->mcode, aoptable[ alink->cdtv[ i ].subset ].opc ) ;
                    if( alink->cdtv[ i ].subset == 4 || alink->cdtv[ i ].subset == 53 || alink->cdtv[ i ].subset == 57)
                    {
                        i++ ;
                        sprintf( reg_adr, "%d0", alink->cdtv[ i ].subset ) ;
                        strcat( alink->mcode, reg_adr ) ;
                    }
                    else if( alink->cdtv[ i ].subset == 36 || alink->cdtv[ i ].subset == 37 )
                    {
                        i++ ;
                        sprintf( reg_adr, "%d", alink->cdtv[ i ].subset + 1 ) ;
                        strcat( alink->mcode, reg_adr ) ;
                        i += 2 ;
                        sprintf( reg_adr, "%d", alink->cdtv[ i ].subset + 1 ) ;
                        strcat( alink->mcode, reg_adr ) ;
                    }
                    else
                    {
                        i++;
                        sprintf( reg_adr, "%d", alink->cdtv[ i ].subset ) ;
                        strcat( alink->mcode, reg_adr ) ;
                        i += 2 ;
                        sprintf( reg_adr, "%d", alink->cdtv[ i ].subset ) ;
                        strcat( alink->mcode, reg_adr ) ;
                    }
                }
                else
                {
                    flag = 0 ;
                    if( alink->cdtv[ i ].subset == 35 )
                    {
                        strcpy( alink->mcode, aoptable[ alink->cdtv[ i ].subset ].opc ) ;
                        strcat( alink->mcode, "0000" ) ;
                    }
                    else if( check == FALSE )
                    {
                        strcpy( alink->mcode, aoptable[ alink->cdtv[ i ].subset ].opc ) ;
                        i++ ;
                        if( alink->cdtv[ i ].set == Sym )
                        {
                            for( j = 0 ; asymtable[ j ].address != FIN ; j++ )
                            {
                                if( asymtable[ j ].sym == atable[ Sym ].command[ alink->cdtv[ i ].subset ] )
                                {
                                    address = asymtable[ j ].address ;
                                    break ;
                                }
                                else
                                    address = FIN ;
                            }
                            if( address == FIN )
                                return FALSE ;
                            else
                            {
                                i++ ;
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 0 )
                                {
                                    direct = 1 ;
                                    address += direct * 32768 ;
                                    sprintf( reg_adr, "%x", address ) ;
                                }
                                else
                                {
                                    i-- ;
                                    sprintf( reg_adr, "%x", address ) ;
                                }
                                strcat( alink->mcode, reg_adr ) ;
                            }
                        }
                    }
                    else
                    {
                        sscanf( aoptable[ alink->cdtv[ i ].subset ].opc, "%x", &opc ) ;
                        if( alink->cdtv[ i - 1 ].set == Del && alink->cdtv[ i - 1 ].subset == 1 )
                        {
                            i++ ;
                            if( alink->cdtv[ i ].set == Sym || alink->cdtv[ i ].set == IR )
                            {
                                opc += 3 ;
                                format4( atable, asymtable, alink, opc, &i ) ;
                            }
                            else
                            {
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 11 )
                                    opc += 1 ;
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 12 )
                                    opc += 2 ;
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 10 )
                                    opc += 3 ;
                                i++ ;
                                format4( atable, asymtable, alink, opc, &i ) ;
                            }
                        }
                        else
                        {
                            i++ ;
                            if( alink->cdtv[ i ].set == Sym || alink->cdtv[ i ].set == IR )
                            {
                                opc += 3 ;
                                format3( atable, asymtable, alink, opc, &i, pc, breg ) ;
                            }
                            else
                            {
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 11 )
                                    opc += 1 ;
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 12 )
                                    opc += 2 ;
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 10 )
                                    opc += 3 ;
                                i++ ;
                                format3( atable, asymtable, alink, opc, &i, pc, breg ) ;
                            }
                        }
                    }
                }
            }
            else if( alink->cdtv[ i ].set == Sym && strcmp( atable[ Sym ].command[ alink->cdtv[ i ].subset ], "BASE" ) == 0 )
            {
                i++ ;
                for( j = 0 ; asymtable[ j ].address != FIN ; j++ )
                {
                    if( asymtable[ j ].sym == atable[ Sym ].command[ alink->cdtv[ i ].subset ] )
                    {
                        address = asymtable[ j ].address ;
                        break ;
                    }
                    else
                        address = FIN ;
                }
                breg = address ;
            }
        }
        if( alink->next != NULL )
            alink = alink->next ;
        else
            break ;
    }
}

int format4( table *atable, symtable *asymtable, tvlink *alink, int opc, int *i )
{
    int j ;
    int address, flag = 0 ;
    int index = 8, base = 4, prog = 2, format = 1 ;
    char reg_adr[ MAX_LEN ], sflag[ MAX_LEN ] ;
    sprintf( alink->mcode, "%02x", opc ) ;
    flag += format ;
    if( alink->cdtv[ *i ].set == Sym || alink->cdtv[ *i ].set == IR )
    {
        if( alink->cdtv[ *i ].set == Sym )
        {
            for( j = 0 ; asymtable[ j ].address != FIN ; j++ )
            {
                if( asymtable[ j ].sym == atable[ alink->cdtv[ *i ].set ].command[ alink->cdtv[ *i ].subset ] )
                {
                    address = asymtable[ j ].address ;
                    break ;
                }
                else
                    address = FIN ;
            }
            if( address == FIN )
                return FALSE ;
            ( *i )++ ;
            if( alink->cdtv[ *i ].set == Del && alink->cdtv[ *i ].subset == 0 )
            {
                flag += index ;
            }
            else
                ( *i )-- ;
        }
        else
            sscanf( atable[ IR ].command[ alink->cdtv[ *i ].subset ], "%x", &address ) ;
    }
    else if( alink->cdtv[ *i ].set == Del && alink->cdtv[ *i ].subset == DH )
    {
        ( *i )++ ;
        for( j = 0 ; asymtable[ j ].address != FIN ; j++ )
        {
            if( atable[ alink->cdtv[ *i ].set ].command[ alink->cdtv[ *i ].subset ] == asymtable[ j ].sym )
            {
                address = asymtable[ j ].address ;
                break ;
            }
        }
        ( *i )++ ;
        if( alink->cdtv[ *i ].set == Del && alink->cdtv[ *i ].subset == 0 )
            return FALSE ;
        else
            ( *i )-- ;
    }
    sprintf( sflag, "%x", flag ) ;
    strcat( alink->mcode, sflag ) ;
    strcat( alink->mcode, "0" ) ;
    sprintf( reg_adr, "%04x", address ) ;
    strcat( alink->mcode, reg_adr ) ;

    return TRUE ;
}

int format3( table *atable, symtable *asymtable, tvlink *alink, int opc, int *i, int pc, int breg )
{
    int j ;
    int address, flag = 0, disp ;
    int index = 8, base = 4, prog = 2, format = 1 ;
    char reg_adr[ MAX_LEN ], sflag[ MAX_LEN ] ;
    sprintf( alink->mcode, "%02x", opc ) ;
    if( alink->cdtv[ *i ].set == Sym || alink->cdtv[ *i ].set == IR )
    {
        if( alink->cdtv[ *i ].set == Sym )
        {
            for( j = 0 ; asymtable[ j ].address != FIN ; j++ )
            {
                if( asymtable[ j ].sym == atable[ Sym ].command[ alink->cdtv[ *i ].subset ] )
                {
                    address = asymtable[ j ].address ;
                    break ;
                }
                else
                    address = FIN ;
            }
        }
        else
            sscanf( atable[ IR ].command[ alink->cdtv[ *i ].subset ], "%x", &address ) ;
        printf( "%x %x\n", pc, address ) ;
        if( address == FIN )
            return FALSE ;
        else
        {
            ( *i )++ ;
            flag += prog ;
            disp = address - pc ;
            if( disp < 0 )
                disp += 4096 ;
            if( disp < 0 )
            {
                flag -= prog ;
                if( breg == FIN )
                    return FALSE ;
                else
                {
                    flag += base ;
                    disp = address - base ;
                }
            }
            if( alink->cdtv[ *i ].set == Del && alink->cdtv[ *i ].subset == 0 )
                flag += index ;
            else
                ( *i )-- ;
            }

    }
    else if( alink->cdtv[ *i ].set == Del && alink->cdtv[ *i ].subset == DH )
    {
        ( *i )++ ;
        if( alink->cdtv[ *i ].subset == Sym )
        {
            for( j = 0 ; asymtable[ j ].address != FIN ; j++ )
            {
                if( atable[ alink->cdtv[ *i ].set ].command[ alink->cdtv[ *i ].subset ] == asymtable[ j ].sym )
                {
                    address = asymtable[ j ].address ;
                    break ;
                }
            }
        }
        else
            sscanf( atable[ IR ].command[ alink->cdtv[ *i ].subset ], "%x", &address ) ;
        ( *i )++ ;
        flag += prog ;
        disp = address - pc ;
        if( disp < 0 )
            disp += 4096 ;
        if( disp < 0 )
        {
            flag -= prog ;
            if( breg == FIN )
                return FALSE ;
            else
            {
                flag += base ;
                disp = address - base ;
            }
        }
        if( alink->cdtv[ *i ].set == Del && alink->cdtv[ *i ].subset == 0 )
            return FALSE ;
        else
            ( *i )-- ;
    }
    sprintf( sflag, "%d", flag ) ;
    strcat( alink->mcode, sflag ) ;
    sprintf( reg_adr, "%03x", disp ) ;
    strcat( alink->mcode, reg_adr ) ;
}
