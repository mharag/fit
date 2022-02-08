/*          Miroslav Harag            */
/*             xharag02               */
/*           FIT VUT Brno             */
/* IZP - praca s datovymi strukturami */
/*             5.12.2020              */


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>

#define TRY(foo) if(!foo) return false;
#define CHECK_ALLOC(ptr, errMsg) if(ptr == NULL){printf("%s", errMsg); return false;}
#define NAN LDBL_MAX

//Konfiguracia
#define MAX_COMM 1000
#define NUM_OF_TVARS 15 //Pocet docasnych textovych premennych
#define MAX_COMM_LEN 1000
#define SIZE_STEP 100 

typedef struct {
    char ***data;
    int *col;//skutocna pozicia stlpca v "data" 
    int *NEsumRow, *NEsumCol;//pocet neprazdych buniek v riadkoch resp. stpcoch
    int dim[2];
} TABLE;    
typedef struct {
    int rows[2];
    int cols[2];
} SELECTION;
typedef struct {
    char *string[NUM_OF_TVARS];
    SELECTION sel;
} TEMP_VARS;
typedef struct {
    TABLE *table;
    SELECTION sel;
    TEMP_VARS *tVars;
    
    char outputSep;
    bool inputSep[256];
} SPREADSHEET;
typedef struct{
    char *name;
    bool (*impl)();
    char *corrArg;
    int internConfig;
    int userI[4];      
    char *userS;
} COMM;

TABLE *tableCTOR();
bool tableDTOR(TABLE *table);
bool enlargeTab(TABLE *t);
void minSize(TABLE *t, int (*size)[2]);
char *read(TABLE *t, int row, int col);
long double readINT(TABLE *t, int row, int col);
bool write(TABLE *t, char *value, int row, int col);
bool swapRows(TABLE *t, int row1, int row2);
bool swapCols(TABLE *t, int col1, int col2);
bool swapCells(TABLE *t, int s1, int s2, int d1, int d2);

bool writeStrVar(TEMP_VARS *vars, char *value, int id);
char *readStrVar(TEMP_VARS *vars, int id);
bool incVar(TEMP_VARS *vars, int id);
bool subVar(TEMP_VARS *vars, int id1, int id2);

SPREADSHEET *spreadSheetCTOR();
bool spreadSheetDTOR(SPREADSHEET *s);
bool loadTable(SPREADSHEET *t, char *source);
bool writeTable(SPREADSHEET *t, char *source);

bool selLine(SPREADSHEET *t, COMM *comm);
bool selRct(SPREADSHEET *t, COMM *comm);
bool selectMin(SPREADSHEET *t, COMM *comm);
bool selectMax(SPREADSHEET *t, COMM *comm);
bool selectText(SPREADSHEET *t, COMM *comm);
bool set(SPREADSHEET *t, COMM *comm);
bool clear(SPREADSHEET *t, COMM *comm);
bool swap(SPREADSHEET *t, COMM *comm);
bool sum(SPREADSHEET *t, COMM *comm);
bool avg(SPREADSHEET *t, COMM *comm);
bool count(SPREADSHEET *t, COMM *comm);
bool len(SPREADSHEET *t, COMM *comm);
bool setTempVar(int name, char *value);

bool icol(SPREADSHEET *t, COMM *c);
bool acol(SPREADSHEET *t, COMM *c);
bool dcol(SPREADSHEET *t, COMM *c);
bool irow(SPREADSHEET *t, COMM *c);
bool arow(SPREADSHEET *t, COMM *c);
bool drow(SPREADSHEET *t, COMM *c);

bool defTempVar(SPREADSHEET *t, COMM *a);
bool useTempVar(SPREADSHEET *t, COMM *a);
bool incTempVar(SPREADSHEET *t, COMM *a);
bool subTempVar(SPREADSHEET *t, COMM *a);
bool defSelVar(SPREADSHEET *t, COMM *a);
bool useSelVar(SPREADSHEET *t, COMM *a);

bool defVar(SPREADSHEET *t, COMM *a);
bool useVar(SPREADSHEET *t, COMM *a);
bool defSel(SPREADSHEET *t, COMM *a);
bool useSel(SPREADSHEET *t, COMM *a);
bool inc(SPREADSHEET *t, COMM *a);

COMM commands[] = {
    {.name = "selectLine", .impl = selLine,    .corrArg = "[%A,%A]"},
    {.name = "selectRct",  .impl = selRct,     .corrArg = "[%A,%A,%A,%A]"},
    {.name = "selectMin",  .impl = selectMin,  .corrArg = "[min]"},
    {.name = "selectMax",  .impl = selectMax,  .corrArg = "[max]"},
    {.name = "selectText", .impl = selectText, .corrArg = "[find %S]"},
    {.name = "set",        .impl = set,        .corrArg = "set %S"},
    {.name = "clear",      .impl = set,        .corrArg = "clear"},
    {.name = "swap",       .impl = swap,       .corrArg = "swap [%I,%I]"},
    {.name = "sum",        .impl = sum,        .corrArg = "sum [%I,%I]"},
    {.name = "avg",        .impl = avg,        .corrArg = "avg [%I,%I]"},
    {.name = "count",      .impl = count,      .corrArg = "count [%I,%I]"},
    {.name = "len",        .impl = len,        .corrArg = "len [%I,%I]"},
    {.name = "icol",       .impl = icol,       .corrArg = "icol",          .internConfig = 0},
    {.name = "acol",       .impl = icol,       .corrArg = "acol",          .internConfig = 1},
    {.name = "dcol",       .impl = dcol,       .corrArg = "dcol"},         
    {.name = "irow",       .impl = irow,       .corrArg = "irow",          .internConfig = 0},
    {.name = "arow",       .impl = irow,       .corrArg = "arow",          .internConfig = 1},
    {.name = "drow",       .impl = drow,       .corrArg = "drow"},
    {.name = "defTempVar", .impl = defTempVar, .corrArg = "def _%I"},
    {.name = "useTempVar", .impl = useTempVar, .corrArg = "use _%I"},
    {.name = "defSelVar",  .impl = defSelVar,  .corrArg = "[set]"},
    {.name = "useSelVar",  .impl = useSelVar,  .corrArg = "[_]"},
    {.name = "incTempVar", .impl = incTempVar, .corrArg = "inc _%I"},
    {.name = "subTempVar", .impl = subTempVar, .corrArg = "sub _%I _%I"},
    {.name = "goto",       .impl = NULL,       .corrArg = "goto %I"},
    {.name = "iszero",     .impl = NULL,       .corrArg = "iszero _%I %I"},
};
char *errMsg[] = {
    [0]"Nepodarilo sa nacitat vstupny subor.\n",
    [1]"Nepodarilo sa ulozit tabulku da pamate PC.\n",
    [2]"Chybne zadane prikazy \n",
    [3]"Maximalny pocet prikazov je 1000\n",
    [4]"Zle zadane parametre.\n",
    [5]"Nezname cislo docasnej premennej.\n",
    [6]"Zle zadane parametre. Spravny tvar [-d delim] CMD_SEQUENCE FILE \n",
    [7]"K dispozicii su iba premenne _0 az _10 \n",
    [8]"Prikaz goto odkazuje na neexistujuci prikaz\n"
};



/* 
 * Praca s tabulkou                            
 *                                            
 * Definovane su operacie - citanie bunky,    
 * zapisovanie do bunky, vymena dvoch riadkov    
 * vymena dvoch stlpcov a vymena buniek      
 */ 

TABLE *tableCTOR(){
    TABLE *table = malloc(sizeof(TABLE));
    
    table->dim[1] = table->dim[0] = 0;
    table->data = NULL;
    table->col = NULL;
    table->NEsumRow = NULL;
    table->NEsumCol = NULL;

    return table;
}
bool tableDTOR(TABLE *table){
    (void) table;
    
    for(int i = 0; i < table->dim[0]; i++){
	for(int j = 0; j < table->dim[1]; j++){
	    free(table->data[i][j]); 
	}
	free(table->data[i]);
    }
    free(table->data);
    free(table->col);
    free(table->NEsumRow);
    free(table->NEsumCol);
   
    free(table);
    return true;
}
/*
 * Zvacsi vyhradene miesto pre tabulku
 * "krok zvacsenia" - SIZE_STEP je nastavitelny
 * TODO: nezavysle zvacsenie sirky a vysky
 */
bool enlargeTab(TABLE *t){
    printf("Nastavujem velkost tabulky na %d x %d\n", t->dim[0]+SIZE_STEP, t->dim[0]+SIZE_STEP);

    //nastavi pocet riadkov
    t->data = realloc(t->data, sizeof(char*)*(t->dim[0]+SIZE_STEP));
    CHECK_ALLOC(t->data, errMsg[1])

    for(int i = t->dim[0]; i < t->dim[0]+SIZE_STEP; i++)
	t->data[i] = NULL;
    t->dim[0] += SIZE_STEP;
    

    //nastavi pocet stlpcov
    for(int i = 0; i < t->dim[0]; i++){
	t->data[i] = realloc(t->data[i], sizeof(char*)*(t->dim[1]+SIZE_STEP));
	CHECK_ALLOC(t->data[i], errMsg[1])
    }

    //vytvori predvolene odkazy na stlpce 
    t->col = realloc(t->col, sizeof(int)*(t->dim[1]+SIZE_STEP));
    CHECK_ALLOC(t->col, errMsg[1])

    for(int i = t->dim[1]; i < t->dim[1]+SIZE_STEP; i++){
	t->col[i] = i;
    }
    t->dim[1] += SIZE_STEP;	
    
    //initializacia na NULL
    for(int i = 0; i < t->dim[0]-SIZE_STEP; i++)
	for(int j = t->dim[1]-SIZE_STEP; j < t->dim[1]; j++)
	    t->data[i][j] = NULL;                            
    for(int i = t->dim[0]-SIZE_STEP; i < t->dim[0]; i++)
	for(int j = 0; j < t->dim[1]; j++)
	    t->data[i][j] = NULL;


    t->NEsumRow = realloc(t->NEsumRow, sizeof(int)*(t->dim[0]+SIZE_STEP)); 
    t->NEsumCol = realloc(t->NEsumCol, sizeof(int)*(t->dim[1]+SIZE_STEP)); 
    //initializacia NEsumRow a NEsumCol
    for(int i = t->dim[0]-SIZE_STEP; i < t->dim[0]; i++){
	t->NEsumRow[i] = 0;
    }
    for(int i = t->dim[0]-SIZE_STEP; i < t->dim[0]; i++){
	t->NEsumCol[i] = 0;
    }

    return true;
} 
/*
 *Do "size" ulozi minimalnu velkost tabulky
 */
void minSize(TABLE *t, int (*size)[2]){
    int i;
    for(i = t->dim[0]-1; i > 0; i--){
	if(t->NEsumRow[i] > 0){
	    break;
	}
    }
    (*size)[0] = i+1;

    for(i = t->dim[1]-1; i > 0; i--){
	if(t->NEsumCol[i] > 0){
	    break;
	}
    }
    (*size)[1] = i+1;
}
/*
 * Vrati textovu hodnotu bunky [row, col]
 * prazdna bunka = ""
 * neexistujuca bunka = NULL
 * cislovanie od jedna 
 */
char *read(TABLE *t, int row, int col){
    if(row > t->dim[0] || col > t->dim[1])
	return NULL;
    else
	return t->data[row-1][t->col[col-1]];
}
/*
 * Vrati ciselnu hodnotu bunky [row, col]
 * prazdna bunka = ""
 * neexistujuca bunka = NULL
 * cislovanie od jedna 
 */
long double readINT(TABLE *t, int row, int col){
    if(row > t->dim[0] || col > t->dim[1])
	return NAN;
    else{
	char *strValue = t->data[row-1][t->col[col-1]];
	char *endptr;
	if(strValue != NULL){
	    double num = strtod(strValue, &endptr);
	    if(*endptr == '\0' && endptr != strValue){
		return num;
	    }
	    else
		return NAN;
	}
	else
	    return NAN; 
    }
}
/*
 * Zapise hodnotu do bunky [row, col]
 * v pripade neuspechu return false;
 * cislovanie od jedna
 */
bool write(TABLE *t, char *value, int row, int col){
    while(row > t->dim[0] || col > t->dim[1]){
	TRY(enlargeTab(t));
    }
	
    char **cell = &t->data[row-1][t->col[col-1]];
    
    bool original = (*cell == NULL)?0:1;
    bool new = (value == NULL || value[0] == '\0')?0:1; 
    if(!original && new){
	t->NEsumRow[row-1]++;
	t->NEsumCol[col-1]++;
    }
    else if(original && !new){
	t->NEsumRow[row-1]--;
	t->NEsumCol[col-1]--;

    }
    
    if(value == NULL || value[0] == '\0'){
	free(*cell);
	*cell = NULL;
    } 
    else{
	int len = strlen(value);
	free(*cell);
	*cell = malloc(sizeof(char)*(len+1)); 
	strcpy(*cell, value);
    }
     
    return true;
}
bool swapRows(TABLE *t, int row1, int row2){
    while(row1 > t->dim[0] || row2 > t->dim[1]){
	enlargeTab(t);
    }

    char **temp = t->data[row1-1];
    t->data[row1-1] = t->data[row2-1];
    t->data[row2-1] = temp; 

    int temp1 = t->NEsumRow[row1-1];
    t->NEsumRow[row1-1] = t->NEsumRow[row2-1];
    t->NEsumRow[row2-1] = temp1;

    return true;
}
bool swapCols(TABLE *t, int col1, int col2){
    while(col1 > t->dim[1] || col2 > t->dim[1]){
	enlargeTab(t);
    }
    
    int temp = t->col[col1-1];
    t->col[col1-1] = t->col[col2-1];
    t->col[col2-1] = temp;

    temp = t->NEsumCol[col1-1];
    t->NEsumCol[col1-1] = t->NEsumCol[col2-1];
    t->NEsumCol[col2-1] = temp;

    return true;
}
bool swapCells(TABLE *t, int s1, int s2, int d1, int d2){
    while(s1 > t->dim[0] || s2 > t->dim[1] || d1 > t->dim[0] || d2 > t->dim[1]){
	TRY(enlargeTab(t));
    }
    char **source = &t->data[s1-1][t->col[s2-1]];    
    char **destination = &t->data[d1-1][t->col[d2-1]];    

    bool sourceEmpty = (*source == NULL || *source[0] == '\0')?0:1;
    bool destEmpty = (*destination == NULL || *destination[0] == '\0')?0:1; 
    if(!sourceEmpty && destEmpty){
	t->NEsumRow[s1-1]++;
	t->NEsumCol[s2-1]++;
	t->NEsumRow[d1-1]--;
	t->NEsumCol[d2-1]--;
    }
    else if(sourceEmpty && !destEmpty){
	t->NEsumRow[s1-1]--;
	t->NEsumCol[s2-1]--;
	t->NEsumRow[d1-1]++;
	t->NEsumCol[d2-1]++;
    }

    char *temp = *source;
    *source = *destination;
    *destination = temp;


    return true;
}



/* Praca s docasnymi premennymi                      
 *                                                   
 * K dispozicii je NUM_OF_TVARS textovych premennych 
 * a jedna premenna na ulozenie akt. vyberu          
 * Definovane su operacie - citanie premennej        
 * ulozenie hodnoty do premennej,                    
 * navysenie num. hodnoty o 1                        
 */

TEMP_VARS *tempVarsCTOR(){
    TEMP_VARS *tVars = malloc(sizeof(TEMP_VARS));
    for(int i = 0; i < NUM_OF_TVARS; i++){
	tVars->string[i] = NULL;
    }
    tVars->sel.rows[0] = 1;
    tVars->sel.rows[1] = 1;
    tVars->sel.cols[0] = 1;
    tVars->sel.cols[1] = 1;

    return tVars; 
}
bool tempVarsDTOR(TEMP_VARS *tVars){
    for(int i = 0; i < NUM_OF_TVARS; i++){
	free(tVars->string[i]);
    }
    free(tVars);
    return true;
}
bool writeStrVar(TEMP_VARS *vars, char *value, int id){
    if(id < 0 || id >= NUM_OF_TVARS){
	printf("%s", errMsg[7]);	
	return false;
    }
    
    free(vars->string[id]);
    
    int len = strlen(value);
    vars->string[id] = malloc(sizeof(char)*(len+1));
    CHECK_ALLOC(vars->string[id], errMsg[1])

    strcpy(vars->string[id], value);
    return true;
}
char *readStrVar(TEMP_VARS *vars, int id){
    if(id >= 0 && id < NUM_OF_TVARS)
	return vars->string[id];
    return NULL;
}
bool incVar(TEMP_VARS *vars, int id){
    //kontrola udajov
    if(id < 0 || id >= NUM_OF_TVARS){
	printf("%s", errMsg[7]);	
	return false;
    }
    
    //zistenie vyslednej hodnoty
    int result = 1;
    if(vars->string[id] != NULL){
	char *endptr;
	int num = strtol(vars->string[id], &endptr, 10);
	if(*endptr == '\0' && endptr != vars->string[id])
	    result = num+1;
    } 

    //prepisanie obsahu premennej
    free(vars->string[id]);	
    vars->string[id] = malloc(sizeof(char)*50);
    CHECK_ALLOC(vars->string[id], errMsg[1])
    snprintf(vars->string[id], 50, "%d", result);

    return true;
}
bool subVar(TEMP_VARS *vars, int id1, int id2){
    if(id1 >= 0 && id1 <= NUM_OF_TVARS && id2 >= 0 && id2 <= NUM_OF_TVARS){
	if(vars->string[id1] != NULL && vars->string[id2] != NULL){
	    char *endptr1, *endptr2;
	    int num1 = strtol(vars->string[id1], &endptr1, 10);
	    int num2 = strtol(vars->string[id2], &endptr2, 10);
	    
	    if(*endptr1 == '\0' && endptr1 != vars->string[id1]){
		if(*endptr2 == '\0' && endptr2 != vars->string[id2]){
		    free(vars->string[id1]);	
	    
		    vars->string[id1] = malloc(sizeof(char)*50);
		    CHECK_ALLOC(vars->string[id1], errMsg[1])
		    snprintf(vars->string[id1], 50, "%d", num1-num2);
		}
 	    }
	}
    } 
    return true;
}



/*
 * Zakladne fukcie pre pracu so SPREADSHEET
 *
 * Struktura SPREADSHEET obsahuje - tabulku pre ukladanie dat
 * 				  - docasne premenne pre ukladanie dat a selekcie   
 *				  - "vyznacene" bunky, ktore maju byt dalej spracovane
 *				  - znaky ktore su chapane ako oddelovace buniek
 */

SPREADSHEET *spreadSheetCTOR(){
    SPREADSHEET *s = malloc(sizeof(SPREADSHEET));
    CHECK_ALLOC(s, errMsg[1])

    s->table = tableCTOR();
    s->tVars = tempVarsCTOR();

    //initializacia selekcie
    s->sel.rows[0] = s->sel.rows[1] = 1;
    s->sel.cols[0] = s->sel.cols[1] = 1;

    //initializacia delim
    for(int i = 0; i < 256; i++){
	s->inputSep[i] = false;
    }
    s->inputSep[32] = true;
    s->outputSep = ' ';


    return s;
}
bool spreadSheetDTOR(SPREADSHEET *s){
    (void) s;
   
    tableDTOR(s->table); 
    tempVarsDTOR(s->tVars);	
    
    free(s); 
    return true;
}
char *fileReadCell(FILE *file, int *end, bool *sep){

    //initializacia bufferu
    char *buffer;
    buffer = malloc(SIZE_STEP*sizeof(char));
    if(buffer == NULL){
	printf("%s", errMsg[1]);
	return NULL;
    }
    int bufSize = SIZE_STEP, bufPos = 0;
    buffer[0] = '\0';
    

    //nacitavanie zo suboru
    int c; 
    bool bounded = false;
    while((c = fgetc(file)) != EOF){
	//ohranicenie bunky uvodzovkami - prvy znak musi byt "	
	if(bufPos == 0 && c == '"'){
	    bounded = true;
	    if((c = fgetc(file)) == EOF) break;
	}

	//escape sequence
	bool ignore = false;
	if(c == '\\'){
	    if((c = fgetc(file)) == EOF) break;
	    ignore = true;
	}

	if(!ignore){
	    
	    //koniec bunky
	    if(bounded){//ohranicene uvodzovkami
		if(c == '"'){
		    c = fgetc(file); 
		    break;
		}
	    }
	    else if(c == '\n')//koniec riadu
		break;
	    else//ukoncene delimetrom
		if(sep[c])
		    break;
	}

	//ulozi vstup do bufferu
	if(bufPos+1 > bufSize){//Rozsirovanie bufferu
	    buffer = realloc(buffer, bufSize + SIZE_STEP);
	    bufSize += SIZE_STEP;
	}
	buffer[bufPos++] = (char)c;
    }	

    buffer[bufPos] = '\0';
    *end = c;
    return buffer;
}
bool loadTable(SPREADSHEET *t, char *source){
    //nacitanie suboru
    printf("Nacitavam vstupny subor\n");
    FILE *file;
    file = fopen(source, "r");
    if(file == NULL){
	printf("%s", errMsg[0]);
	return false;
    }

    char *value;
    int end;
    int row = 1, col = 1;
    while((value = fileReadCell(file, &end, t->inputSep)) != NULL){
	write(t->table, value, row, col);
	free(value);
	
	if(end == EOF){
	    break;
	}	
	else if(end == '\n'){
	    col = 1; 
	    row++;
	}
	else{
	    col++;
	}
    }

    fclose(file);
    return true;
}
bool writeTable(SPREADSHEET *t, char *source){
    printf("Zapisujem tabulku do suboru.\n");

    //otvori subor do ktoreho ma byt tabulka zapisana
    FILE *file;
    file = fopen(source, "w");
    if(file == NULL){
	printf("%s", errMsg[0]);
	return false;
    }
    
    //zisti rozmery tabulky
    int size[2];
    minSize(t->table, &size);   

    //vypise tabulku
    char *buf;
    for(int i = 1; i <= size[0]; i++){
	for(int j = 1; j <= size[1]; j++){
	    //prida oddelovac pred kazdu bunku (okrem prvej)
	    if(j != 1) fprintf(file, "%c", t->outputSep);
	   
	    //nacita bunku [i,j] z tabulky  
	    if((buf = read(t->table, i, j))!= NULL){
		//hladanie specialnych znakov v bunke
		bool clean = true;
		for(int k = 0; buf[k]; k++)
		    if(buf[k] == '\\' || buf[k] == '"' || t->outputSep == buf[k]) 
			clean = false;

		//vypise jednu bunku
		if(clean)//bunka ktora neobsahuje specialne znaky
		    fprintf(file, "%s", buf);
		else{//bunka so specialnymi znakmi 
		    fprintf(file, "\"");
		    for(int k = 0; buf[k]; k++)
			if(buf[k] == '\\' || buf[k] == '"')
			    fprintf(file, "\\%c", buf[k]);
			else
			    fprintf(file, "%c", buf[k]);
		    fprintf(file, "\"");
		}
	    }
	}
	//ukoncenie riadku
	fprintf(file, "\n");
    }

    fclose(file);
    return true;
}



/* Rozsirujuce funkcie pre pracu so SPREADSHEET
 *
 * Su definove funkcie pre - zmenu vyberu buniek
 * 			   - ulozenie a citanie s docasnych premennych
 * 			   - zmenu udajov v bunkah aktualneho vyberu
 * 			   - pridavanie/odstranovanie celych stlpcov a riadkov
 * 			   - pocitanie suctu, priemeru, dlzky... buniek akt. vyberu
 *			   - nastavenie delimetru
 */

//zmena vyberu buniek
bool selLine(SPREADSHEET *t, COMM *comm){
    int size[2];
    minSize(t->table, &size);
    
    t->sel.rows[0] = comm->userI[0]==INT_MAX?1:comm->userI[0];//row start
    t->sel.rows[1] = comm->userI[0]==INT_MAX?size[0]:comm->userI[0];//row end
    t->sel.cols[0] = comm->userI[1]==INT_MAX?1:comm->userI[1];// col start
    t->sel.cols[1] = comm->userI[1]==INT_MAX?size[1]:comm->userI[1];//row end
    
    if(t->sel.rows[0] < 1 || t->sel.cols[0] < 1){
	printf("%s", errMsg[4]);
	return false;
    }
    return true;
}
bool selRct(SPREADSHEET *t, COMM *comm){
    int size[2];
    minSize(t->table, &size);
    
    t->sel.rows[0] = comm->userI[0];//row start
    t->sel.rows[1] = comm->userI[2]==INT_MAX?size[0]:comm->userI[2];//row end
    t->sel.cols[0] = comm->userI[1];// col start
    t->sel.cols[1] = comm->userI[3]==INT_MAX?size[1]:comm->userI[3];//col end
    
    if(t->sel.rows[0] < 1 || t->sel.cols[0] < 1 
    || t->sel.rows[0] > t->sel.rows[1] 
    || t->sel.cols[0] > t->sel.cols[1]){
	printf("%s", errMsg[4]);
	return false;
    }
    return true;
}
bool selectMin(SPREADSHEET *t, COMM *c){
    (void) c;
    int result[2] = {1,1};
    double temp = DBL_MAX;
    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++)
	for(int j = t->sel.cols[0]; j <= t->sel.cols[1]; j++){
	    long double num = readINT(t->table, i, j);
	    if(num != NAN)
		if(temp > num){
		    temp = num;
		    result[0] = i;
		    result[1] = j;
		}
	}
    t->sel.rows[0] = t->sel.rows[1] = result[0];
    t->sel.cols[0] = t->sel.cols[1] = result[1];
    return true;
}
bool selectMax(SPREADSHEET *t, COMM *c){
    (void) c;
    int result[2] = {1,1};
    double temp = DBL_MIN;
    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++)
	for(int j = t->sel.cols[0]; j <= t->sel.cols[1]; j++){
	    long double num = readINT(t->table, i, j);
	    if(num != NAN)
		if(temp < num){
		    temp = num;
		    result[0] = i;
		    result[1] = j;
		}
	}
    t->sel.rows[0] = t->sel.rows[1] = result[0];
    t->sel.cols[0] = t->sel.cols[1] = result[1];
    return true;
}
bool selectText(SPREADSHEET *t, COMM *c){
    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++){
	for(int j = t->sel.cols[0]; j <= t->sel.cols[1]; j++){
	    if(strstr(read(t->table, i, j), c->userS) != NULL){
		t->sel.rows[0] = t->sel.rows[1] = i;
		t->sel.cols[0] = t->sel.cols[1] = j;
		return true;
	    }
	}
    }
    return true;
}
//zmena obsahu buniek
bool set(SPREADSHEET *t, COMM *c){
    if(c->internConfig == 1)
	c->userS = NULL;

    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++){
	for(int j = t->sel.cols[0]; j <= t->sel.cols[1]; j++){
	    TRY(write(t->table, c->userS, i, j))
	}
    }	
    return true; 
}
bool swap(SPREADSHEET *t, COMM *c){
    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++){
	for(int j = t->sel.cols[0]; j <= t->sel.cols[1]; j++){
	    swapCells(t->table, i, j, c->userI[0], c->userI[1]);
	}
    }
    return true;
}
bool sum(SPREADSHEET *t, COMM *c){
    double sum = 0;
    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++){
	for(int j = t->sel.cols[0]; j <= t->sel.cols[1]; j++){
	    long double value = readINT(t->table, i, j);
	    if(value != NAN)
		sum += readINT(t->table, i, j);
	}
    }
    char output[50];
    snprintf(output, 50, "%g", sum);
    if(output == NULL){
	return false;
    }
    write(t->table, output, c->userI[0], c->userI[1]);
    return true;
}
bool avg(SPREADSHEET *t, COMM *c){
    double sum = 0;
    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++){
	for(int j = t->sel.cols[0]; j <= t->sel.cols[1]; j++){
	    long double value = readINT(t->table, i, j);
	    if(value != NAN)
		sum += readINT(t->table, i, j);
	}
    }

    double avg = sum / ((t->sel.rows[1]-t->sel.rows[0]+1)*(t->sel.cols[1]-t->sel.cols[0]+1));    

    char output[50];
    snprintf(output, 50, "%g", avg);
    write(t->table, output, c->userI[0], c->userI[1]);
    return true;
}
bool count(SPREADSHEET *t, COMM *c){
    int count = 0;
    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++){
	for(int j = t->sel.cols[0]; j <= t->sel.cols[1]; j++){
	    if(read(t->table, i, j) != NULL && read(t->table,i,j)[0] != '\0'){
		count++;
	    }
	}
    }
    char output[50];
    snprintf(output, 50, "%d", count);
    write(t->table, output, c->userI[0], c->userI[1]);
    return true;
}
bool len(SPREADSHEET *t, COMM *c){
    int len = 0;
    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++){
	for(int j = t->sel.cols[0]; j <= t->sel.cols[1]; j++){
	    if(read(t->table, i, j) != NULL)
		len += strlen(read(t->table, i, j));
	}
    }
    char output[50];
    snprintf(output, 50, "%d", len);
    write(t->table, output, c->userI[0], c->userI[1]);
    return true;
}
//pridavanie/odstranovanie stlpcov a riadkov
bool icol(SPREADSHEET *t, COMM *c){
    (void) *c;
    int position;
    if(c->internConfig == 0)//icol 
	position = t->sel.cols[0];
    else//acol
	position = t->sel.cols[1]+1;

    int size[2];
    minSize(t->table, &size);
    for(int i = size[1]+1; i > position; i--){
	swapCols(t->table, i, i-1);
    }

    return true;
}
bool dcol(SPREADSHEET *t, COMM *c){
    (void)c;
    
    int size[2];
    minSize(t->table, &size);

    for(int i = t->sel.cols[0]; i <= t->sel.cols[1]; i++){
	for(int j = 1; j <= size[0]; j++){
	    write(t->table, "", j, i);
	}
    }
    int deleted = t->sel.cols[1] - t->sel.cols[0] + 1;
    for(int i = t->sel.cols[1]+1 ; i <= size[1]; i++){
	swapCols(t->table, i, i-deleted);
    }

    return true;
}
bool irow(SPREADSHEET *t, COMM *c){
    (void) *c;
    int position;
    if(c->internConfig == 0)//irow 
	position = t->sel.rows[0];
    else//arow
	position = t->sel.rows[1]+1;

    int size[2];
    minSize(t->table, &size);
    for(int i = size[0]+1; i > position; i--){
	swapRows(t->table, i, i-1);
    }
    minSize(t->table, &size);

    return true;
}
bool drow(SPREADSHEET *t, COMM *c){
    (void)c;
    
    int size[2];
    minSize(t->table, &size);

    for(int i = t->sel.rows[0]; i <= t->sel.rows[1]; i++){
	for(int j = 1; j <= size[1]; j++){
	    write(t->table, "", i, j);
	}
    }
    int deleted = t->sel.rows[1] - t->sel.rows[0] + 1;
    for(int i = t->sel.rows[1]+1 ; i <= size[0]; i++){
	swapRows(t->table, i, i-deleted);
    }

    return true;
}
//ulozenie a citanie s docasnych premennych
bool defTempVar(SPREADSHEET *s, COMM *c){
    int varID = c->userI[0];
    char *value = read(s->table, s->sel.rows[0], s->sel.cols[0]);

    return writeStrVar(s->tVars, value, varID);
}
bool useTempVar(SPREADSHEET *s, COMM *c){
    int varID = c->userI[0];

    if(varID < 0 || varID> 9){
	printf("%s", errMsg[7]);
	return false;
    } 

    char *value = readStrVar(s->tVars, varID);
    for(int i = s->sel.rows[0]; i <= s->sel.rows[1]; i++){
	for(int j = s->sel.cols[0]; j <= s->sel.cols[1]; j++){
	    write(s->table, value, i, j);
	}
    }
    
    return true;
}
bool incTempVar(SPREADSHEET *s, COMM *c){
    int varID = c->userI[0];

    if(varID < 0 || varID> 9){
	printf("%s", errMsg[7]);
	return false;
    } 

    incVar(s->tVars, varID);
    
    return true;
}
bool subTempVar(SPREADSHEET *s, COMM *c){
    int varID1 = c->userI[0];
    int varID2 = c->userI[1];

    if(varID1 < 0 || varID1 > 9 || varID2 < 0 || varID2 > 9){
	printf("%s", errMsg[7]);
	return false;
    } 

    subVar(s->tVars, varID1, varID2);

    return true;
}
bool defSelVar(SPREADSHEET *s, COMM *c){
    (void) c;
    s->tVars->sel = s->sel;
    return true;    
}
bool useSelVar(SPREADSHEET *s, COMM *c){
    (void) c;
    s->sel = s->tVars->sel;
    return true;    
}
//nastavenie delimetru
bool setDelimeter(SPREADSHEET *t, char *value){
    for(int i = 0; value[i]; i++) 
	t->inputSep[(int)value[i]] = true; 
    t->outputSep = value[0]; 
    return true;
}



/*
 * Spracovanie prikazov od uzivatela
 *
 */

int scanVar(char *str, COMM *actComm){
    char *pattern = actComm->corrArg;
    int i = 0, j = 0, resPos = 0;
    while(pattern[j])
	if(!str[i]) return 0;//str je moc kratke
	else if(pattern[j] == '%'){//podobne ako scanf
	    j++;//preskocenie percenta
	    if(pattern[j] == 'I' || pattern[j] == 'A')//I -> integer, A -> integer + '_' 
		if(pattern[j] == 'A' && (str[i] == '_' || str[i] == '-')){
		    actComm->userI[resPos++] = INT_MAX;
		    i++;
		}
		else{
		    char *end;
		    long temp = strtol(str+i, &end, 10);
		    if(end == str+i || temp < INT_MIN || temp > INT_MAX)//ak nie je cislo
			return 0;
		    actComm->userI[resPos++] = temp;
		    i += end-str-i;//preskocenie uz nacitaneho cisla
		}
	    else if(pattern[j] == 'S'){//S -> string
	    
		//initializacia bufferu
		char *buffer;
		buffer = malloc(SIZE_STEP*sizeof(char));
		if(buffer == NULL){
		    printf("%s", errMsg[1]);
		    return 0;
		}
		int bufSize = SIZE_STEP, bufPos = 0;
		buffer[0] = '\0';
		

		//nacitavanie zo suboru
		bool bounded = false;
		if(str[i] == '"'){
		    bounded = true;
		    i++;
		}
		while(str[i]){

		    //escape sequence
		    bool ignore = false;
		    if(str[i] == '\\'){
			if(!str[++i]) break;
			ignore = true;
		    }

		    if(!ignore){
			//koniec bunky
			if(bounded){//ohranicene uvodzovkami
			    if(str[i] == '"'){
				i++;
				break;
			    }
			}
			else//ukoncene delimetrom
			    if(str[i] == pattern[j+1] || str[i] == ';')
				break;
		    }

		    //ulozi vstup do bufferu
		    if(bufPos+1 > bufSize){//Rozsirovanie bufferu
			buffer = realloc(buffer, bufSize + SIZE_STEP);
			bufSize += SIZE_STEP;
		    }
		    buffer[bufPos++] = str[i++];
		}	

		buffer[bufPos] = '\0';
		actComm->userS = buffer;
	    }
	    j++;
	}
	else
	    if(str[i++] != pattern[j++])
		return 0;
    return i;
}
bool decodeComms(COMM *a, int *commSize, char *str){
    printf("Dekodujem zadane prikazy\n");

    int sPos = 0;
    while(str[sPos]){
	if(str[sPos] == ';' || str[sPos] == ' ') sPos++;
	int found = false;
	COMM actComm;
	for(int i = 0; i < (int)(sizeof(commands)/sizeof(COMM)); i++){
	    actComm = commands[i];
	    int shift = 0;
	    if((shift = scanVar(str+sPos, &actComm))){
		sPos += shift;
		found = true;
		if(*commSize >= MAX_COMM){
		    printf("%s", errMsg[3]);
		    return false;
		}	
		a[(*commSize)++] = actComm;
		break;
	    }
	}
	if(!found){
	    printf("%s", errMsg[2]);
	    return false;
	}
	if(str[sPos] != ';')
	    break;	
    }
    if(!str[sPos])
	return true;
    else{
	printf("%s", errMsg[2]);
	return false;
    }
}
bool executeComms(SPREADSHEET *t, COMM *a, int commSize){
    printf("Vykonavam prikazy\n");

    for(int i = 0; i < commSize; i++){		
	
	//riadiace prikazy - goto
	if(!strcmp(a[i].name, "goto")){
	    if(i+a[i].userI[0] >= 0 && i+a[i].userI[0] < commSize){
		i = i+a[i].userI[0]-1;
		continue;
	    }
	    else{
		printf("%s", errMsg[8]);
		return false;
	    }
	}
    
	//riadiace prikazy - sub	
	if(!strcmp(a[i].name, "iszero")){
	    if(i+a[i].userI[1] >= 0 && i+a[i].userI[1] < commSize){
		char *tempVar = readStrVar(t->tVars, a[i].userI[0]);
		char *endptr;
		long num = strtol(tempVar, &endptr, 10);
		if(endptr != tempVar && *endptr == '\0' && num == 0)
		    i = i+a[i].userI[1]-1;
		continue;
	    }
	    else{
		printf("%s", errMsg[8]);
		return false;
	    }
	}

	//obycajne prikazy
	else{
	    TRY(a[i].impl(t, &a[i]))
	}
    } 
    return true;
}



int main(int argc, char **argv){

    char *delim = " ";//predvoleny delim
    if(!strcmp(argv[1], "-d")){
	if(argc != 5){
	    printf("%s", errMsg[6]);
	    return -1;
	}
	else
	    delim = argv[2];
    }
    else if(argc != 3){
	printf("%s", errMsg[6]);
	return -1;
    } 

    SPREADSHEET *t = spreadSheetCTOR();  

    int commSize = 0;
    COMM *comms = malloc(1000*sizeof(COMM));//pole prikazov

    int success = false;
    if(setDelimeter(t, delim)                      
    && decodeComms(comms, &commSize, argv[argc-2]) 
    && loadTable(t, argv[argc-1])                  
    && executeComms(t, comms, commSize)            
    && writeTable(t, argv[argc-1]))
	success = true;

    //cistenie
    spreadSheetDTOR(t);
    for(int i = 0; i < commSize; i++){
	free(comms[i].userS);
    }
    free(comms);

    
    if(success) 
	return 0;
    else 
	return -1;
}	
