/*          Miroslav Harag            */
/*             xharag02               */
/*           FIT VUT Brno             */
/*       IZP - Praca s textom         */
/*            15.11.2020              */

#include <stdio.h>
#include <stdbool.h> 
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_S_SIZE 101 //maximalna podporovana dlzka stringu 
#define MAX_DIM 10240 //maximalna velkost riadku
#define TRY(COMM) if(!(COMM)) return false; 

typedef struct {
    bool (*impl)();//odkaz na implmantaciu daneho prikazu
    char name[MAX_S_SIZE];
    char *corr_arg;//pozadovany format argumentov
    int intern_config;//rovnaka funkcia moze implementovat viacero prikazov - potrebujeme ich rozlisit
} COMM;//prikaz od uzivatela w()
typedef struct {
    COMM comm; 
    int usr_i[4];//parametre od uzivatela(integer)
    char *usr_s;//parametre od uzivatela(string)
    int temp_res;//funkcie rmax, rmin... zavisia na predoslych vysledkoch funkcie
    int prefix[100]; //pre kmp 
} COMM_SETUP;//Konkretne nastavenie 
typedef struct {
    char line[2][MAX_DIM+3];//uzivatelsky vstup - parne riadky sa ukladaju do line[0], neparne do line[1]
    int num;//cislo riadku
    bool last;
    int storage;
} ROW;//riadok vstupu
typedef struct {
    int start;//zaciatok n-tej bunky na aktualnom riadku
    int end;//koniec buknky
    char value[101];//hodnota bunky
    bool exist;
} CELL;//bunka v riadku
typedef struct {
    bool input_sep[256]; //input_sep[i] urcuje ci je (char)i brany ako oddelovac
    char output_sep; //oddelovac popuzity na vystupe
} DELIM;//delimeter pretransformovany do pola pre rychlejsie zistovanie ci je znak delim
typedef struct {
    int start; //jednoduche prikazy irow icol... budu implementovane rovnako ako drows dcols
    int end; //  (bude pre ne platit start = end)
} _T_ARG; //jeden argument pre upravu tabulky napr icol, drows...
typedef struct {
    _T_ARG changes[4][100];//zoradene vsetky upravy tabulky od najmensej. Postupne irows drows icols dcols
    int size[4];//pocet jednotlivych uprav tabulky
    int empty_end[2];//pocet prazdnych riadkov/stlpcov na konci
} TABLE_EDIT;//Informacie o zmene tabulky
typedef struct {
    int c;//argc
    char **v;//argv
    int pos;//zaciatok parametrov ktore nas aktualne zaujimaju  
} USER_ARG;

bool proces_arg(int argc, char **argv);//spracuje vsetky argmunety, vysledkom je hotova tabulka na vystupe
bool set_sep(DELIM *delim, USER_ARG *arg);//nastavi delimiter 
bool check_arg(int argc, char **argv, char *correct_format);//skontroluje ci maju argumenty spravny format
bool comm_init(COMM_SETUP *set, COMM comm, USER_ARG *arg);//nastavi vsetko potrebne pre prikaz 
int find_width(DELIM delim, char *line);
bool data_process(DELIM *delim, COMM_SETUP set, COMM_SETUP sel);//zakladana funkcia pre spracovanie dat
bool translate_com(TABLE_EDIT *table, COMM comm, USER_ARG *arg);//prelozi prikaz na upravy tabulky do TABLE_EDIT
bool generate_table(DELIM delim, TABLE_EDIT table);//vygeneruje tabulku podla TABLE_EDIT
bool print_line(DELIM delim, char line[], TABLE_EDIT table, int input_width);//vypise jeden riadok tabulky podla TABLE_EDIT
bool kmp_init(int *prefix, char *pattern);
bool kmp(char *text, char *pattern, int *prefix);
bool rows_sel(DELIM *delim, ROW *row, COMM_SETUP set);//vrati true ak riadok vyhovuje selekcii riadkov inak false
bool find_cell(DELIM *delim, char *line, CELL *cell, int start, int col, char *print);//najde bunku v riadku 
bool cset(DELIM *delim, ROW *row, COMM_SETUP *set);
bool cedit(DELIM *delim, ROW *row, COMM_SETUP *set);//editacia obsahu bunky - prikazy:tolower, toupper, int, round
bool copy(DELIM *delim, ROW *row, COMM_SETUP *set);
bool swap(DELIM *delim, ROW *row, COMM_SETUP *set);
bool move(DELIM *delim, ROW *row, COMM_SETUP *set);
bool c(DELIM *delim, ROW *row, COMM_SETUP *set);//zahrnuje prikazy cmax, cmin, cavg, ccount
bool r(DELIM *delim, ROW *row, COMM_SETUP *set);//zahrnuje prikazy cmax, cmin, cavg, ccount
bool cseq(DELIM *delim, ROW *row, COMM_SETUP *set);
bool rseq(DELIM *delim, ROW *row, COMM_SETUP *set);

char *error_msg[] = {//chybove hlasky
    [0]"Je podporovany maximalne jeden prikaz pre spracovanie dat naraz!\n",
    [1]"Nie je mozne kombinovat prikazy pre spracovanie dat a prikazy pre editaciu tabulky!\n",
    [2]"Neznamy prikaz %s!\n",
    [3]"Selekciu riadkov nie je mozne kombinovat s prikazmi pre editaciu tabulky!\n",
    [4]"Nedostatocny pocet parametrov pre prikaz %s!\n",
    [5]"%d. parameter prikazu %s musi byt cele cislo z rozsahu %s\n",
    [6]"Maximalna podporovana dlzka stringu je 100!\n",
    [7]"Prvy parameter pre prikaz %s nemoze byt vacsi ako druhy.",
    [8]"Prikazy pre upravu tabulky maju kolizne cisla!\n",
    [9]"\nVelkost jednej bunky je obmedzena na 100 znakov!\n",
    [10]"Maximalny pocet prikazov %s %s je 100!\n", //(pridavajucich/odstranujucih, riadky/stlpce)
};
//prikazy su oddelene do skupin aby sa dalo skontrolovat ci sa nemiesaju zakazane prikazy
COMM comms_rows_sel[] = {//prikazy pre selekciu riadkov
    {.name = "rows"       , .corr_arg = "SAA" , .intern_config = 0},//vyznam corr_arg je napisany pri funkcii check_arg
    {.name = "beginswith" , .corr_arg = "SNS" , .intern_config = 1},
    {.name = "contains"   , .corr_arg = "SNS" , .intern_config = 2}
};
COMM comms_data[] = {//prikazy pre spracovanie dat
    {.impl = &cset  , .name = "cset"    , .corr_arg = "SNS"   , .intern_config = 0},//ja si to uzivam, ja sa citim dobre
    {.impl = &cedit , .name = "tolower" , .corr_arg = "SN"    , .intern_config = 0},
    {.impl = &cedit , .name = "toupper" , .corr_arg = "SN"    , .intern_config = 1},
    {.impl = &cedit , .name = "round"   , .corr_arg = "SN"    , .intern_config = 2},
    {.impl = &cedit , .name = "int"     , .corr_arg = "SN"    , .intern_config = 3},
    {.impl = &copy  , .name = "copy"    , .corr_arg = "SNN"   , .intern_config = 0},
    {.impl = &swap  , .name = "swap"    , .corr_arg = "SNN"   , .intern_config = 1},
    {.impl = &move  , .name = "move"    , .corr_arg = "SNN"   , .intern_config = 2},
    {.impl = &c     , .name = "csum"    , .corr_arg = "SNNN"  , .intern_config = 0},
    {.impl = &c     , .name = "cavg"    , .corr_arg = "SNNN"  , .intern_config = 1},
    {.impl = &c     , .name = "cmin"    , .corr_arg = "SNNN"  , .intern_config = 2},
    {.impl = &c     , .name = "cmax"    , .corr_arg = "SNNN"  , .intern_config = 3},
    {.impl = &c     , .name = "ccount"  , .corr_arg = "SNNI"  , .intern_config = 4},
    {.impl = &cseq  , .name = "cseq"    , .corr_arg = "SNNN"  , .intern_config = 0},
    {.impl = &r     , .name = "rsum"    , .corr_arg = "SNNN"  , .intern_config = 0},
    {.impl = &r     , .name = "ravg"    , .corr_arg = "SNNN"  , .intern_config = 1},
    {.impl = &r     , .name = "rmin"    , .corr_arg = "SNNN"  , .intern_config = 2},
    {.impl = &r     , .name = "rmax"    , .corr_arg = "SNNN"  , .intern_config = 3},
    {.impl = &r     , .name = "rcount"  , .corr_arg = "SNNN"  , .intern_config = 4},
    {.impl = &rseq  , .name = "rseq"    , .corr_arg = "SNNNI" , .intern_config = 0}
};
COMM comms_table[] = {//prikazy pre editaciu tabulky
    {.name = "irow"    , .corr_arg = "SN"  , .intern_config = 0},
    {.name = "icol"    , .corr_arg = "SN"  , .intern_config = 1},
    {.name = "drow"    , .corr_arg = "SN"  , .intern_config = 2},
    {.name = "dcol"    , .corr_arg = "SN"  , .intern_config = 3},
    {.name = "drows"   , .corr_arg = "SNN" , .intern_config = 4},
    {.name = "dcols"   , .corr_arg = "SNN" , .intern_config = 5},
    {.name = "arow"    , .corr_arg = "S"   , .intern_config = 6},
    {.name = "acol"    , .corr_arg = "S"   , .intern_config = 7}
};

int main(int argc, char **argv){
    if(!proces_arg(argc, argv))
	return -1;   
    return 0;
}
bool proces_arg(int argc, char **argv){
    DELIM delim = {.input_sep = {}};
    bool default_delim = true;

    int mode = 0; // 1 uprava tabulky 2 spracovanie dat
    COMM_SETUP set;//nastavenia pre data_process prikaz (mode 2)
    COMM_SETUP sel = {.comm.intern_config = -1};//nastavenia pre rows_sel prikaz (mode 2)
    TABLE_EDIT table = {.size = {1, 1, 1, 1}}; //nastavenia pre editaciu tabulky (mode 1)    
    for(int i = 0; i < 4; i++)//inicializacia table.changes
	table.changes[i][0].end = table.changes[i][0].start = INT_MAX;//oznacuje koniec pola changes[i]

    USER_ARG arg = {.c = argc, .v = argv, .pos = 1};
    while(arg.pos < arg.c){
	bool unknown = true;//neznamy prikaz
	for(int j = 0; j < (int)(sizeof(comms_data)/sizeof(COMM)); j++)//prehladava comms_data[]
	    if(!strcmp(arg.v[arg.pos], comms_data[j].name)){
		unknown = false;
		if(mode == 2){//chyba - maximalne jeden prikaz v mode 2
		    printf(error_msg[0]);
		    return 0;
		}
		else if(mode == 1){//chyba - uz bol nastaveny mode 1
		    printf(error_msg[1]);
		    return 0;
		}
		TRY(comm_init(&set, comms_data[j], &arg))//nastavi prikaz comms_data[j] do premennej set aby sa neskor mohol vykonat
		mode = 2;
		break;
	    }
	if(!unknown) continue;
	for(int j = 0; j < (int)(sizeof(comms_table)/sizeof(COMM)); j++){//
	    if(!strcmp(arg.v[arg.pos], comms_table[j].name)){//prehladava comms_table[]
		unknown = false;
		if(mode == 2){//chyba - uz bol nastaveny mod 2
		    printf(error_msg[1]);
		    return 0;
		}
		TRY(translate_com(&table, comms_table[j], &arg))//pozadovanu zmenu tabulky zapise do table
		mode = 1;
		break;
	    }
	}
	if(!unknown) continue;
	for(int j = 0; j < (int)(sizeof(comms_rows_sel)/sizeof(COMM)); j++)//prehlada comms_rows_sel[]
	    if(!strcmp(arg.v[arg.pos], comms_rows_sel[j].name)){
		unknown = false;
		TRY(comm_init(&sel, comms_rows_sel[j], &arg))//nastavi premennu sel
		if(sel.comm.intern_config == 2)
		    kmp_init(sel.prefix, sel.usr_s);
		break;
	    }
	if(!unknown) continue;
	if(!strcmp(arg.v[arg.pos], "-d")){//nastavi separator
	    default_delim = false;
	    TRY(set_sep(&delim, &arg))
	    unknown = false;
	}
	if(unknown){//chyba - neznamy prikaz
	    printf(error_msg[2], arg.v[arg.pos]);
	    return false;
	}
    }

    if(default_delim){//ak nebol delim nastaveny explicitne nastavi predvolenu hodnotu
	delim.input_sep[' '] = true;
	delim.output_sep = ' ';
    }

    if(mode == 2){//spracovanie hodnot
	TRY(data_process(&delim, set, sel))
    }
    else if (mode == 1){//editacia tabulky
	if(sel.comm.intern_config != -1){//chyba - rows sel spolu s editaciou tabulky
	    printf(error_msg[3]);
	    return 0;
	}
	generate_table(delim, table);
    }
    else//ziadna zmena = editacia tabulky bez zmeny
	generate_table(delim, table);
    return true; //uspesne ukoncenie
}
bool set_sep(DELIM *delim, USER_ARG *arg){//nastavi delim
    TRY(check_arg(arg->c - arg->pos, arg->v + arg->pos, "SS")) //skontroluje ci su spravne parametre pre -d
    for(int j = 0; arg->v[arg->pos+1][j]; j++)
	delim->input_sep[(int)arg->v[arg->pos+1][j]] = true;
    delim->output_sep = arg->v[arg->pos+1][0];
    arg->pos = arg->pos + 2;//preskumali sme dva argumenty -d "", ktore nas uz dalej nezaujimaju
    return true;
}
bool check_arg(int argc, char **argv, char *correct_format){
    for(int i = 0; correct_format[i]; i++)
	if(i >= argc){
            printf(error_msg[4], argv[0]);
            return false;
	}
	else{
	    char *end;
	    long num = strtol(argv[i], &end, 10);
	    bool is_number = (end != argv[i] && *end == '\0');
	    switch(correct_format[i]){
		case 'N': //natural - prirodzene cislo
		    if(!is_number || num < 1 || num > INT_MAX){ 
			printf(error_msg[5], i, argv[0], "<1, INT_MAX>");
			return false;
		    }
		    break;
		case 'I': //integer - cele cislo
		    if(!is_number || num < INT_MIN || num > INT_MAX){ 
			printf(error_msg[5], i, argv[0], "<INT_MIN, INT_MAX>");
			return false;
		    }
		    break;
		case 'A': //advanced - prirodzene cislo a specialny znak '-'
		    if((!is_number || num < 1 || num > INT_MAX) && strcmp(argv[i], "-")){ 
			printf(error_msg[5], i, argv[0], "<INT_MIN, INT_MAX> alebo znak -");
			return false;
		    }
		    break;
		case 'S':; //string
		    if(strlen(argv[i]) > 100){
			printf(error_msg[6]);
			return false;
		    }
		    break;
		}
	}
    return true;
}
bool comm_init(COMM_SETUP *set, COMM comm, USER_ARG *arg){
    TRY(check_arg(arg->c - arg->pos, arg->v + arg->pos, comm.corr_arg))//Skontroluje argumenty
    set->comm = comm;
    for(int j = 0, i = 0; comm.corr_arg[j]; j++){
	if(comm.corr_arg[j] == 'S')
	    set->usr_s = arg->v[arg->pos];
	else
	    if(arg->v[arg->pos][0] == '-')//osetruje znak '-' pre rows pre typ ADVANCED
		set->usr_i[i++] = INT_MAX;
	    else
		set->usr_i[i++] = strtol(arg->v[arg->pos], NULL, 10);
	arg->pos++;
    }
    return true;
}
int find_width(DELIM delim, char *line){
    int width = 1;
    for(int i = 0; line[i]; i++)
	if(delim.input_sep[(int)line[i]])
	    width++;
    return width;
}
//Editacia tabulky
bool translate_com(TABLE_EDIT *table, COMM comm, USER_ARG *arg){
    TRY(check_arg(arg->c - arg->pos, arg->v + arg->pos, comm.corr_arg))//Skontroluje argumenty
    arg->pos++;//preskakuje nazov prikazu
    int start, end, mode;
    if(comm.intern_config < 6){//vsetky prikazy okrem acol arow
	if(comm.intern_config < 4){//jednoduche prikazy s jednim parametrom
	    end = start = strtol(arg->v[arg->pos++], NULL, 10);
	    mode = comm.intern_config;
	}
	else{//drows dcols
	    start = strtol(arg->v[arg->pos++], NULL, 10);
	    end = strtol(arg->v[arg->pos++], NULL, 10);
	    if(start > end){//chyba - zle arg od uzivatela
		printf(error_msg[7], comm.name);
		return false;
	    }
	    mode = comm.intern_config-2;
	}
	if(table->size[mode]==100){//chyba - prekroceny max prikazov
	    printf(error_msg[10], mode<1?"pridavajucich":"odstranujucich", mode%2?"stlpce":"riadky");
	    return false;
	}
	int i = 0;//hladanie spravnej pozicie v table->changes
	while(start >= table->changes[mode][i].start)
	    i++;
	for(int j = table->size[mode]; j > i; j--)//uvolnenie miesta
	    table->changes[mode][j] = table->changes[mode][j-1];
	if((i > 0 && table->changes[mode][i-1].end >= start) || (table->changes[mode][i].start <= end)){//chyba - kolizne cisla
	    printf(error_msg[8]);
	    return false;
	}
	table->changes[mode][i].start = start;
	table->changes[mode][i].end = end;
	table->size[mode]++;
    }
    else//specialne prikazy bez parametrov (arow, acol)
	table->empty_end[comm.intern_config-6]++;
    return true;
}
bool generate_table(DELIM delim, TABLE_EDIT table){
    //zisti pociatocnu sirku tabulky
    ROW row = {.num = 1, .storage = 0};
    TRY(fgets(row.line[row.storage], MAX_DIM, stdin))
    int input_width = find_width(delim, row.line[row.storage]);
    int tracker_i = 0;
    int tracker_d = 0;
    do{
	if(row.num>table.changes[0][tracker_i].end)
	    tracker_i++;
	if(row.num>table.changes[2][tracker_d].end)
	    tracker_d++;
	if(table.changes[0][tracker_i].start <= row.num)//vypise prazdny riadok
	    print_line(delim, "", table, input_width);
	if(table.changes[2][tracker_d].start > row.num)//vypise riadok ak neni vymazany
	    print_line(delim, row.line[row.storage], table, input_width);
	row.num++;
    }while(fgets(row.line[row.storage], MAX_DIM, stdin));
    for(int i = 0; i < table.empty_end[0]; i++)//vypise prazdne riadky na konci
	print_line(delim, "", table, input_width);
    return true;
}
bool print_line(DELIM delim, char line[], TABLE_EDIT table, int input_width){
    int trc_i = 0;//tracker insert
    int trc_d = 0;//tracker delete
    int input_pos = 0;
    bool first = true;
    for(int i = 1; i < input_width+1; i++){
	if(i>table.changes[1][trc_i].end)
	    trc_i++;
	if(i>table.changes[3][trc_d].end)
	    trc_d++;
	bool insert = table.changes[1][trc_i].start <= i;
	bool delete = table.changes[3][trc_d].start <= i;
	if(insert)//vypise prazdnu bunku - oddelovac
	    printf("%c", delim.output_sep);
	if(!delete){//vypise oddelovac medzi bunky
	    if(!first)
		printf("%c", delim.output_sep);
	    first = false;
	}
	while(line[input_pos]){//vypise obsah bunky - podporuje neobmedzenu velkost bunky
	    if(delim.input_sep[(int)line[input_pos]] || line[input_pos] == '\n'){
		input_pos++;
		break;
	    }
	    if(!delete)
		printf("%c", line[input_pos]);
	    input_pos++;
	}
    }
    for(int i = 0; i < table.empty_end[1]; i++)//vypise prazdne stlpce na koniec
	printf("%c", delim.output_sep);
    printf("\n");
    return true;
}
//Selekcia riadkov
bool kmp_init(int *prefix, char *pattern){
    prefix[0] = -1;
    for(int i = 0; pattern[i]; i++){
        int corr = prefix[i];
        while(corr >= 0 && pattern[i] != pattern[corr])
            corr = prefix[corr];
        prefix[i+1]=corr+1;
    }
    return true;
}
bool kmp(char *text, char *pattern, int *prefix){//Knuth-Morris-Prattov algoritmus
    int pattern_pos = 0;
    for(int i = 0; text[i]; i++){
        while(pattern_pos >= 0 && pattern[pattern_pos] != text[i])
            pattern_pos = prefix[pattern_pos];
        pattern_pos++;
        if(!pattern[pattern_pos])
	    return true;
    }
    return false;
}
bool rows_sel(DELIM *delim, ROW *row, COMM_SETUP set){
    CELL cell;
    switch(set.comm.intern_config){
	case 0://rows
	    if(row->num >= set.usr_i[0] && row->num <= set.usr_i[1])
		return true;
	    else if(INT_MAX == set.usr_i[0] && INT_MAX == set.usr_i[1] && row->last)//pripad rows - -
		return true;
	    return false;
	case 1://begins
	    find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set.usr_i[0], "000");
	    return strncmp(set.usr_s, cell.value, strlen(set.usr_s)) == 0;
	case 2://contains - treba dokoncit(prefixy)
	    find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set.usr_i[0], "000");
	    return kmp(cell.value, set.usr_s, set.prefix);
    }
    return true;
}
//Spracovanie dat - povinne
bool data_process(DELIM *delim, COMM_SETUP set, COMM_SETUP sel){
    CELL cell;
    ROW row = {.last = false};
    fgets(row.line[0], MAX_DIM, stdin);//parne riadky su ulozene v row.line[0], neparne v row.line[1]
    for(row.num = 1; fgets(row.line[row.num%2], MAX_DIM, stdin); row.num++){//spracuje vsetky riadky okrem posledneho
	if(!rows_sel(delim, &row, sel))//kontroluje ci ma byt riadok spracovany
	    find_cell(delim, row.line[(row.num-1)%2], &cell, 0, 1, "111");
	else
	    TRY(set.comm.impl(delim, &row, &set))
	printf("\n");
    }
    row.last = true;//spracuje posledny riadok
    if(!rows_sel(delim, &row, sel))
	find_cell(delim, row.line[(row.num-1)%2], &cell, 0, 1, "111");
    else
	TRY(set.comm.impl(delim, &row, &set))
    printf("\n");
    return true;
}
bool find_cell(DELIM *delim, char *line, CELL *cell, int start, int col, char *print){
    //najde i-tu bunku od pozicie start v riadku line, (print - ak je 1. znak '1' vypise sa vsetko pred najdenou bunkou, 
    //ak 2. vypise sa bunka, ak 3. vypise sa vsetko za bunkou
    int i = start;
    while(col > 1){//najde zaciatok bunky
        if(!line || line[i] == '\n'){
	    cell->exist = false;
            return true;
        }
        if(delim->input_sep[(int)line[i]])
            col--;
        if(print[0] == '1')
            printf("%c", delim->input_sep[(int)line[i]]?delim->output_sep:line[i]);
        i++;
    }
    cell->start = i;    
    int j = 0;
    while(line[i] && !delim->input_sep[(int)line[i]] && line[i] != '\n'){//vypise/ulozi obsah bunky
        if(j > 99){
            printf(error_msg[9]);
            return false;
        } 
        if(print[1] == '1')
            printf("%c", line[i]);
        cell->value[j++] = line[i++];
    }
    cell->value[j] = '\0';
    cell->end = i;
    if(print[2] == '1')//vypise zvysok riadku
	while(line[i] && line[i] != '\n'){            
	    printf("%c", delim->input_sep[(int)line[i]]?delim->output_sep:line[i]);
        i++;
    }
    cell->exist = true;
    return true;
}
bool cset(DELIM *delim, ROW *row, COMM_SETUP *set){
    CELL cell;
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set->usr_i[0], "100"))//vypise vsetko pred bunkou
    if(cell.exist){//ak nebolo v riadku dost stlpcov prikaz sa odignoruje
	printf("%s", set->usr_s);
	TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, cell.start, 0, "001"))//vypise vsetko za bunkou
    }
    return true;
}
bool cedit(DELIM *delim, ROW *row, COMM_SETUP *set){
    CELL cell; 
    char *end;
    double num;
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set->usr_i[0], "100"))//vypise vsetko pred bunkou
    if(cell.exist){
	switch(set->comm.intern_config){
	    case 0://tolower
		for(int i = 0; cell.value[i]; i++)
		    printf("%c", tolower(cell.value[i]));
		break;
	    case 1://toupper
		for(int i = 0; cell.value[i]; i++)
		    printf("%c", toupper(cell.value[i]));
		break;
	    case 2://round
		num = strtof(cell.value, &end);
		if(end != cell.value && *end == '\0')
		    printf("%.0f", num);
		else
		    printf("%s", cell.value);
		break;
	    case 3://int
		num = strtof(cell.value, &end);
		if(end != cell.value && *end == '\0')
		    printf("%d", (int)num);
		else
		    printf("%s", cell.value);
		break;
	}
	TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, cell.start, 0, "001"))//vypise vsetko za bunkou
    }
    return true;
}
bool copy(DELIM *delim, ROW *row, COMM_SETUP *set){
    CELL cell_n, cell_m;
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell_n, 0, set->usr_i[0], "000"))//zisti hodnotu bunky n
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell_m, 0, set->usr_i[1], "100"))//vypise vsetko pred bunkou m
    if(cell_m.exist){
	printf("%s", cell_n.exist?cell_n.value:cell_m.value);//vypise obsah bunky n
	TRY(find_cell(delim, row->line[(row->num-1)%2], &cell_m, cell_m.start, 0, "001"))//vypise vsetko za bunkou m
    }
    return true;
}
bool swap(DELIM *delim, ROW *row, COMM_SETUP *set){
    if(set->usr_i[1] < set->usr_i[0]){//vacsia hodnota bude druha
        int temp = set->usr_i[1];
        set->usr_i[1] = set->usr_i[0];
        set->usr_i[0] = temp;
    }
    CELL cell_n, cell_m;
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell_n, 0, set->usr_i[0], "100"))//vypise vsetko pred bunkou n
    if(cell_n.exist){//iba ak bunka n existuje
	TRY(find_cell(delim, row->line[(row->num-1)%2], &cell_m, cell_n.end, set->usr_i[1]-set->usr_i[0]+1, "010"))//vypise obsah bunky m
	if(!cell_m.exist)
	    printf("%s", cell_n.value);//ak bunka m neexistuje - vypise obsah bunky n
	TRY(find_cell(delim, row->line[(row->num-1)%2], &cell_m, cell_n.end, set->usr_i[1]-set->usr_i[0]+1, "100"))
	if(cell_m.exist){
	    printf("%s", cell_n.exist?cell_n.value:cell_m.value);
	    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell_m, cell_m.start, 0, "001"))//vypise zvisok riadku
	}
    }
    return true;
}
bool move(DELIM *delim, ROW *row, COMM_SETUP *set){
    CELL cell_n, cell_m;
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell_n, 0, set->usr_i[0], "000"))//skontroluje ci existuje bunka n
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell_m, 0, set->usr_i[1], "000"))//skontroluje ci existuje bunka m
    if(set->usr_i[1] == set->usr_i[0] || !cell_n.exist || !cell_m.exist)
	set->usr_i[1] = set->usr_i[0] = INT_MAX;
    int input_pos = 0;
    bool first = true;
    for(int i = 1; row->line[(row->num-1)%2][input_pos]; i++){
	if(i != set->usr_i[0]){//vypise oddelovac medzi bunky
	    if(!first)
		printf("%c", delim->output_sep);
	    first = false;
	}
	if(i == set->usr_i[1]){//vlozi bunku n pred bunku m
	    printf("%s%c", cell_n.value, delim->output_sep);
	    first = false;
	}
	while(row->line[(row->num-1)%2][input_pos]){//vypise zvysne bunky okrem bunky n
	    if(delim->input_sep[(int)row->line[(row->num-1)%2][input_pos]] || row->line[(row->num-1)%2][input_pos] == '\n'){
		input_pos++;
		break;
	    }
	    if(i != set->usr_i[0])
		printf("%c", row->line[(row->num-1)%2][input_pos]);
	    input_pos++;
	}
    }
    return true;
}
//Spracovanie dat - premiove
int init(char *str, int mode){
    switch(mode){
        case 0 ... 3: //sum, avg, min, max
            return strtod(str, NULL);
        case 4: //count
            return str[0]?1:0; 
    }
    return 0;
}
int calc(int res, char *str, int mode){
    switch(mode){
        case 0 ... 1: //sum, avg
            return res + strtod(str, NULL);
        case 2: //min
            return res>strtod(str, NULL)?strtod(str, NULL):res;
        case 3: //max
            return  res<strtod(str, NULL)?strtod(str, NULL):res;
        case 4: //count
            return res + (str[0]?1:0); 
    }
    return 0;
}
bool c(DELIM *delim, ROW *row, COMM_SETUP *set){
    CELL cell;
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set->usr_i[1], "000"))
    int res = init(cell.value, set->comm.intern_config);
    for(int i = 0; i < set->usr_i[2]-set->usr_i[1]; i++){//zisti vyslednu hodnotu
        TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, cell.end+1, 1, "000"))
        res = calc(res, cell.value, set->comm.intern_config);
    }
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set->usr_i[0], "100"))
    if(cell.exist){//vypise vyslednu hodnotu
	if(set->comm.intern_config == 1)
	    printf("%.3f", (float)res/((float)set->usr_i[2]-set->usr_i[1]+1));
	else
	    printf("%d", res);
	TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, cell.start, 0, "001"))
    }
    return true;
}
bool cseq(DELIM *delim, ROW *row, COMM_SETUP *set){
    CELL cell;
    if(find_width(*delim, row->line[(row->num-1)%2]) < set->usr_i[1]){
	TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, 1, "111"))
	return 1;
    }
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set->usr_i[0], "100"))
    for(int i = 0; i < set->usr_i[1]-set->usr_i[0]+1; i++){
	TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, cell.end+1, 1, "000"))
        if(i && cell.exist)
            printf("%c", delim->output_sep);
        printf("%d", set->usr_i[2]+i);
    }
    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, cell.end, set->usr_i[1]-set->usr_i[0]+1, "001"))
    return true;
}
bool r(DELIM *delim, ROW *row, COMM_SETUP *set){
    CELL cell;
    if(row->num == set->usr_i[1]){
        TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set->usr_i[0], "111"))
        set->temp_res = init(cell.value, set->comm.intern_config);
    }
    else if(row->num > set->usr_i[1] && row->num <= set->usr_i[2]){ 
        TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set->usr_i[0], "111"))
        set->temp_res = calc(set->temp_res, cell.value, set->comm.intern_config);
    }
    else if(row->num == set->usr_i[2]+1){
        TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set->usr_i[0], "100"))
	if(cell.exist){
	    if(set->comm.intern_config == 1)
		printf("%g", set->temp_res/((float)set->usr_i[2]-set->usr_i[1]+1));
	    else
		printf("%d", set->temp_res);
	    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, cell.start, 0, "001"))
	}
    }
    else
        TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, 0, "111"))
    return true;    
}
bool rseq(DELIM *delim, ROW *row, COMM_SETUP *set){
    CELL cell;
    if(row->num >= set->usr_i[1] && row->num <= set->usr_i[2]){ 
        TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, set->usr_i[0], "100"))
	if(cell.exist){
	    printf("%d", row->num+set->usr_i[3]-set->usr_i[1]);
	    TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, cell.start, 0, "001"))
	}
    }
    else
        TRY(find_cell(delim, row->line[(row->num-1)%2], &cell, 0, 0, "111"))
    return true;  
}
