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
#define TRUE 1
#define FALSE 0

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
    int loc ;
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

tvlink *createlink( tvlink*, typevalue*, char* ) ;
void freelink( tvlink* ) ;
void bulidoptable( optable*, table* ) ;
int checkxe( optable*, tvlink* ) ;
int pass1( optable*, table*, tvlink*, symtable* ) ;
int pass2( optable*, table*, tvlink*, symtable*, int ) ;
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

int main()
{
    int i, count = 0, check ;
    int xecheck ;
    char fname[ MAX_ROW ], choice[ MAX_LEN ] ;
    tvlink *alink = NULL ;
    table stdtable[ 7 ] ;
    symtable stdsymtable[ MAX_TABLE ] ;
    optable stdoptable[ 59 ] ;
    getTable( "Table1.table", &stdtable[ Ins ] ) ;
    getTable( "Table2.table", &stdtable[ Nins ] ) ;
    getTable( "Table3.table", &stdtable[ Reg ] ) ;
    getTable( "Table4.table", &stdtable[ Del ] ) ;
    initializetable( &stdtable[ Sym ], MAX_TABLE ) ;
    initializetable( &stdtable[ IR ], MAX_TABLE ) ;
    initializetable( &stdtable[ Str ], MAX_TABLE ) ;
    bulidoptable( stdoptable, stdtable ) ;
    printf( "Input 0 to exit, or press another key to continue.\n" ) ;
    scanf("%s", choice ) ;
    while( strcmp( choice, "0" ) != 0 )
    {
        printf( "Please input filename.\n" ) ;
        scanf( "%s", fname ) ;
        alink = bulidtable( fname, stdtable, alink ) ;
        xecheck = checkxe( stdoptable, alink ) ;
        check = pass1( stdoptable, stdtable, alink, stdsymtable ) ;
        if( check == TRUE )
        {
            check = pass2( stdoptable, stdtable, alink, stdsymtable, xecheck ) ;
            for( i = 0 ; stdsymtable[ i ].address != FIN ; i++ )
                printf( "%s %x\n",stdsymtable[ i ].sym, stdsymtable[ i ].address ) ;
            i = 0 ;
            while( alink->next != NULL )
            {
                if( alink->cdtv[ 0 ].set != FIN )
                {
                    printf( "%d %04x\t%s\t\t%s\n", i, alink->loc, alink->code, alink->mcode ) ;
                    i++ ;
                }
                alink = alink->next ;
            }
        }
        else
            printf("XXXXX\n") ;
        printf( "Input 0 to exit, or press another key to continue.\n" ) ;
        scanf( "%s", choice ) ;
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

tvlink *createlink( tvlink *alink, typevalue *cdtv, char *code )
{
    int i ;
    tvlink *newlink, *linkn ;
    newlink = malloc( sizeof( tvlink ) ) ;
    for( i = 0 ; cdtv[ i ].set != FIN ; i++ )
        newlink->cdtv[ i ] = cdtv[ i ] ;
    newlink->cdtv[ i ].set = FIN ;
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
    char infname[ MAX_ROW ], outfname[ MAX_ROW ] = "output_" ;
    int i, j, count ;
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
                alink = createlink( alink, cdtv, code ) ;
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
    if( token[ strlen( token ) - 1 ] >= '0' || token[ strlen( token ) - 1 ] <= '9' )
    {
        for( i = 0 ; i < strlen( token ) - 1 ; i++ )
        {
            if( token[ i ] < '0' || token[ i ] > '9' )
            {
                i = NONE ;
                break ;
            }
        }
        if( i == NONE )
            enarray( cdtv, Sym, hash( token, &stdtable[ Sym ] ), count ) ;
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
    while( alink != NULL )
    {
        int i ;
        for( i = 0 ; alink->cdtv[ i ].set != FIN ; i++ )
        {
            if( alink->cdtv[ i ].set == Ins )
            {
                if( aoptable[ alink->cdtv[ i ].subset ].format == 1 || aoptable[ alink->cdtv[ i ].subset ].format == 2 )
                    return TRUE ;
                else if( alink->cdtv[ i - 1 ].set == Del && alink->cdtv[ i - 1 ].subset == 1 )
                {
                    if( aoptable[ alink->cdtv[ i ].subset ].format == 3 )
                        return TRUE ;
                    else
                        return FIN ;
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

int pass1( optable *aoptable, table *stdtable, tvlink *alink, symtable *asymtable )
{
    int i, loc = 0, symc = 0, count = 0 ;
    int x, j, check, buf ;
    char buffer[ MAX_LEN ] ;
    while( alink != NULL )
    {
        count++ ;
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
                        if( alink->cdtv[ i - 2 ].set == Sym && i >= 2 )
                        {
                            asymtable[ symc ].sym = stdtable[ Sym ].command[ alink->cdtv[ i - 2 ].subset ] ;
                            asymtable[ symc ].address = loc ;
                            symc++ ;
                        }
                    }
                    else if( alink->cdtv[ i ].set == Sym )
                        ;
                    else
                        return FALSE ;
                }
                else if( alink->cdtv[ i - 1 ].subset == 1 )
                {
                    if( alink->cdtv[ i ].set == Sym )
                    {
                        if( ( strcmp( stdtable[ Sym ].command[ alink->cdtv[ i ].subset ], asymtable[ 0 ].sym ) ) == 0 )
                            alink->loc = FIN ;
                    }
                    else
                        return FALSE ;
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
                        i++ ;
                    }
                    else
                        return FALSE ;
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
                        return FALSE ;
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
                        return FALSE ;
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
                        return FALSE ;
                }
                else
                    return FALSE ;
            }
            else if( alink->cdtv[ i ].set == Ins )
            {
                alink->loc = loc ;
                loc += aoptable[ alink->cdtv[ i ].subset ].format ;
                printf( "%x %x %d\n", loc, alink->loc, aoptable[ alink->cdtv[ i ].subset ].format ) ;
                i++ ;
                if( alink->cdtv[ i - 2 ].set == Sym && i >= 2 )
                {
                    asymtable[ symc ].sym = stdtable[ Sym ].command[ alink->cdtv[ i - 2 ].subset ] ;
                    asymtable[ symc ].address = loc ;
                    symc++ ;
                    for( j = 0 ; j < symc - 1 ; j++ )
                    {
                        if( asymtable[ j ].sym == asymtable[ symc - 1 ].sym )
                            return FALSE ;
                    }
                }
                else if( alink->cdtv[ i - 2 ].set == Del && alink->cdtv[ i - 2 ].subset == 1 && i >= 3 )
                {
                    if( alink->cdtv[ i - 3 ].set == Sym )
                    {
                        asymtable[ symc ].sym = stdtable[ Sym ].command[ alink->cdtv[ i - 3 ].subset ] ;
                        asymtable[ symc ].address = loc ;
                        symc++ ;
                        for( j = 0 ; j < symc - 1 ; j++ )
                        {
                            if( asymtable[ j ].sym == asymtable[ symc - 1 ].sym )
                                return FALSE ;
                        }
                    }
                }
                if( aoptable[ alink->cdtv[ i - 1 ].subset ].format == 1 || alink->cdtv[ i - 1 ].subset == 35 )
                {
                    if( alink->cdtv[ i ].set == FIN || alink->cdtv[ i ].set == Ins || alink->cdtv[ i ].set == Nins )
                        i-- ;
                    else
                        return FALSE ;
                }
                else if( alink->cdtv[ i ].set == Reg && aoptable[ alink->cdtv[ i - 1 ].subset ].format == 2 )
                {
                    check = format2check( alink, &i ) ;
                    if( check == FALSE )
                        return FALSE ;
                }
                else if( alink->cdtv[ i ].set == Sym || alink->cdtv[ i ].set == IR )
                {
                    if( alink->cdtv[ i - 2 ].set == Del && alink->cdtv[ i - 2 ].subset == 1 )
                        loc++ ;
                    i++ ;
                    if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 0 )
                    {
                        i++ ;
                        if( alink->cdtv[ i ].set == Reg && alink->cdtv[ i ].subset == 1 )
                            ;
                        else
                            return FALSE ;
                    }
                    else if( alink->cdtv[ i ].set == FIN || alink->cdtv[ i ].set == Ins || alink->cdtv[ i ].set == Nins )
                         i-- ;
                    else
                        return FALSE ;
                }
                else if( alink->cdtv[ i ].set == Reg )
                {
                    i++ ;
                    if( alink->cdtv[ i ].set == FIN || alink->cdtv[ i ].set == Ins || alink->cdtv[ i ].set == Nins )
                         i-- ;
                    else
                        return FALSE ;
                }
                else
                    return FALSE ;
            }
            else if( alink->cdtv[ i ].set == Sym && strcmp( stdtable[ Sym ].command[ alink->cdtv[ i ].subset ], "EQU" ) == 0 )
            {
                i++ ;
                if( alink->cdtv[ i - 2 ].set == Sym && i >= 2 )
                {
                    asymtable[ symc ].sym = stdtable[ Sym ].command[ alink->cdtv[ i - 2 ].subset ] ;
                    if( alink->cdtv[ i ].set == IR )
                    {
                        sscanf( stdtable[ IR ].command[ alink->cdtv[ i ].subset ], "%d", &asymtable[ symc ].address ) ;
                        alink->loc = asymtable[ symc ].address ;
                    }
                    symc++ ;
                    for( j = 0 ; j < symc - 1 ; j++ )
                    {
                        if( asymtable[ j ].sym == asymtable[ symc - 1 ].sym )
                            return FALSE ;
                    }
                }
                else
                    return FALSE ;
            }
        }
        if( alink->next != NULL )
            alink = alink->next ;
        else
            break ;
    }
    asymtable[ symc ].address = FIN ;
    if( alink != NULL )
        return TRUE ;
}

int BWcount( int count, int *loc, symtable *asymtable, int BW, tvlink *alink, table *stdtable, int *symc )
{
    int i ;
    alink->loc = *loc ;
    if( alink->cdtv[ count - 2 ].set == Sym && count  >= 2 )
    {
        asymtable[ *symc ].sym = stdtable[ Sym ].command[ alink->cdtv[ count - 2 ].subset ] ;
        asymtable[ *symc ].address = *loc ;
        ( *symc )++ ;
        for( i = 0 ; i < *symc - 1 ; i++ )
        {
            if( asymtable[ i ].sym == asymtable[ *symc - 1 ].sym )
                return FALSE ;
        }
    }
    *loc += BW ;
    return TRUE ;
}

int format2check( tvlink *alink, int *loc )
{
    if( alink->cdtv[ *loc - 1 ].subset == 4 || alink->cdtv[ *loc - 1 ].subset == 53 || alink->cdtv[ *loc - 1 ].subset == 57 )
    {
        if( alink->cdtv[ *loc + 1 ].set == FIN || alink->cdtv[ *loc + 1 ].set == Ins || alink->cdtv[ *loc + 1 ].set == Nins )
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
                    return TRUE ;
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

int pass2( optable *aoptable, table *atable, tvlink *alink, symtable *asymtable, int check )
{
    int i, j, opc ;
    int pc = 0 ,direct, address, breg = FIN ;
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
                if( aoptable[ alink->cdtv[ i ].subset ].format == 1 || alink->cdtv[ i ].subset == 35 )
                {
                    strcpy( alink->mcode, aoptable[ alink->cdtv[ i ].subset ].opc ) ;
                    strcat( alink->mcode, "0000" ) ;
                }
                else if( aoptable[ alink->cdtv[ i ].subset ].format == 2 )
                {
                    strcpy( alink->mcode, aoptable[ alink->cdtv[ i ].subset ].opc ) ;
                    if( alink->cdtv[ i ].subset == 36 || alink->cdtv[ i ].subset == 37 )
                    {
                        i++;
                        sprintf( reg_adr, "%d", alink->cdtv[ i ].subset + 1 ) ;
                        strcat( alink->mcode, reg_adr ) ;
                        i += 2 ;
                        sprintf( reg_adr, "%d", alink->cdtv[ i ].subset + 1 ) ;
                        strcat( alink->mcode, reg_adr ) ;
                    }
                    else if( alink->cdtv[ i ].subset != 4 && alink->cdtv[ i ].subset != 53 && alink->cdtv[ i ].subset != 57 )
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
                    if( check == FALSE )
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
                            if( alink->cdtv[ i ].set == Sym )
                            {
                                opc += 3 ;
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
                                flag += format ;
                                i++ ;
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 0 )
                                {
                                    flag += index ;
                                }
                                else
                                    i-- ;
                                sprintf( alink->mcode, "%x", opc ) ;
                                sprintf( sflag, "%x", flag ) ;
                                strcat( alink->mcode, sflag ) ;
                                strcat( alink->mcode, "0" ) ;
                                sprintf( reg_adr, "%04x", address ) ;
                                strcat( alink->mcode, reg_adr ) ;
                            }
                            else if( alink->cdtv[ i ].set == IR )
                            {
                                opc += 3 ;
                                address = 0 ;
                                i++ ;
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 0 )
                                    return FALSE ;
                                sprintf( alink->mcode, "%x", opc ) ;
                                sprintf( sflag, "%x", flag ) ;
                                strcat( alink->mcode, "0" ) ;
                                sprintf( reg_adr, "%04x", address ) ;
                                strcat( alink->mcode, reg_adr ) ;
                            }
                        }
                        else
                        {
                            i++ ;
                            if( alink->cdtv[ i ].set == Sym )
                            {
                                opc += 3 ;
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
                                    if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 0 )
                                    {
                                        flag += index ;
                                    }
                                    else
                                        i-- ;
                                    sprintf( alink->mcode, "%x", opc ) ;
                                    sprintf( sflag, "%d", flag ) ;
                                    strcat( alink->mcode, sflag ) ;
                                    sprintf( reg_adr, "%x", disp ) ;
                                    strcat( alink->mcode, reg_adr ) ;
                                }
                            }
                            else if( alink->cdtv[ i ].set == IR )
                            {
                                opc += 3 ;
                                disp = 0 ;
                                i++ ;
                                if( alink->cdtv[ i ].set == Del && alink->cdtv[ i ].subset == 0 )
                                    return FALSE ;
                                else
                                    i-- ;
                                sprintf( alink->mcode, "%x", opc ) ;
                                sprintf( sflag, "%d", flag ) ;
                                strcat( alink->mcode, sflag ) ;
                                sprintf( reg_adr, "%03x", disp ) ;
                                strcat( alink->mcode, reg_adr ) ;
                            }
                        }
                    }
                }
            }
        }
        if( alink->next != NULL )
            alink = alink->next ;
        else
            break ;
    }
}
