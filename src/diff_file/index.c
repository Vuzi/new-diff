#include "index.h"

/* Prototype statique */
static unsigned int index_size(FILE *f);
static t_index* index_file_small(FILE *f, const char* f_name);
static t_index* index_file_large(FILE *f, const char* f_name);

/* ===============================================
                    index_size

    Permet de compter le nombre de lignes du
    fichier f.
    ----------------------------------------------
    FILE *f : fichier à compter.
    ----------------------------------------------
    Retour : Retourne le nombre de lignes de f
   =============================================== */
static unsigned int index_size(FILE *f) {
    unsigned int cpt = 0;
    char c = 0;

    while((c = getc(f)) != EOF) {
        if(c == END_LINE)
            cpt++;
    }
    cpt++; // Dernière ligne

    rewind(f);

    return cpt;
}

void index_file_c_func(t_index* index) {

    unsigned int i = 0, *tmp = NULL;
    char c_tmp = 0;

    index->c_func = (unsigned int*)malloc(sizeof(unsigned int)*(size_t)(index->line_max)); // Taille max

    /* Pour chaques lignes */
    for(i = 0; i < index->line_max; i++) {

        fseek(index->f, index->index[i], SEEK_SET);

        /* Si la ligne commence par un caractère possible */
        if(((c_tmp = (char)getc(index->f)) >= 'a' && c_tmp <= 'z') ||
            (c_tmp >= 'A' && c_tmp <= 'Z') ||
            (c_tmp == '_')){

            index->c_func[index->c_func_nb] = i;
            index->c_func_nb++;
        }
    }

    /* On remet à la bonne taille */
    tmp = (unsigned int*)malloc(sizeof(unsigned int)*((size_t)(index->c_func_nb)));
    memcpy(tmp, index->c_func, (size_t)(index->c_func_nb)*sizeof(unsigned int));
    free(index->c_func);
    index->c_func = tmp;

    rewind(index->f);

}

/* ===============================================
                    index_file

    Permet d'indexer un fichier pour permettre
    ensuite facilement le traitement.
    ----------------------------------------------
    FILE *f        : fichier à indexer.
    c char *f_name : nom du fichier.
    ----------------------------------------------
    Retour : Retourne la structure instanciée pour
             l'nedexation. Renvoit une structure
             avec 0 ligne si le fichier n'est pas
             ouvert ou vide.
   =============================================== */
t_index* index_file(FILE *f, const char* f_name) {

    struct stat st;

    if(stat(f_name, &st) == 0) {
        if(st.st_size >= /*1048576*/ 1) // 1Mo
            return index_file_large(f, f_name);
        else
            return index_file_small(f, f_name);
    } else
        return index_file_large(f, f_name);
}


static t_index* index_file_small(FILE *f, const char* f_name) {

    /* On initialise */
    t_index *new_i = index_file_large(f, f_name);

    unsigned int i = 0;

    new_i->lines = (char**)malloc(sizeof(char*)*(new_i->line_max));

    if(new_i->index) {
        /* Pour les premières lignes */
        for(i = 0; i < new_i->line_max+1; i++) {
            new_i->lines[i] = (char*)calloc(1, sizeof(char)*(new_i->index[i+1]-new_i->index[i]+1)); // alloc
            if(fread(new_i->lines[i], 1, new_i->index[i+1]-new_i->index[i]-1, new_i->f) != new_i->index[i+1]-new_i->index[i]-1) { // recopie
                if(ferror(new_i->f)) {
                    printf("%s : fread() error. Program will try to continue.\n", f_name);
                }
            }
        }
        /* Pour la dernière ligne */
        fseek(new_i->f, 0, SEEK_END);
        new_i->lines[i] = (char*)calloc(1, sizeof(char)*(ftell(new_i->f)-new_i->index[i]+1)); // alloc
        if(fread(new_i->lines[i], 1, ftell(new_i->f)-new_i->index[i]-1, new_i->f) != new_i->index[i+1]-new_i->index[i]-1) { // recopie
            if(ferror(new_i->f)) {
                printf("%s : fread() error. Program will try to continue.\n", f_name);
            }
        }
    }

    rewind(f);

    /* On renvoit la structure */
    return new_i;
}

static t_index* index_file_large(FILE *f, const char* f_name) {

    /* On initialise */
    t_index *new_i = (t_index*)malloc(sizeof(t_index));

    char tmp = 0;
    unsigned int i = 0;

    new_i->f_name = (char*)malloc(sizeof(char)*(strlen(f_name)+1));
    strcpy(new_i->f_name, f_name);

    /* Valeurs de base */
    new_i->f = f;
    new_i->line_max = index_size(new_i->f);
    new_i->lines = NULL;
    new_i->index = (unsigned int*)malloc(sizeof(int)*new_i->line_max);
    new_i->line = 0;
    new_i->c_func = NULL;
    new_i->c_func_nb = 0;

    /* Rapide test pour vérifier si le fichier n'est pas vide */
    if(((tmp = getc(f)) != EOF)) {

        rewind(f);

        /* Premiere ligne */
        new_i->index[i] = 0;
        i++;

        /* Pour chaque char */
        while((tmp = getc(f)) != EOF) {
            /* Si nouvelle ligne */
            if(tmp == END_LINE) {
                /* On rempli la case */
                new_i->index[i] = ftell(new_i->f);
                i++;
            }
        }

    }

    rewind(f);

    /* On renvoit la structure */
    return new_i;
}

/* ===============================================
                    free_index

    Permet de libérer un index.
    ----------------------------------------------
    t_index* index : index à libérer.
   =============================================== */
void index_free(t_index* index) {
    if(index) {
        free(index->index);
        free(index->f_name);
        free(index);
    }
}

/* ===============================================
                  display_index_file

    Affiche le contenu de la structure de f.
    ----------------------------------------------
    t_index *f : fichier à afficher.
   =============================================== */
void index_display(t_index *f) {

    unsigned int i = 0;

    if(f){
        printf("Fichier de %d ligne(s) :\n", f->line_max);

        for(i = 0; i+1 < f->line_max; i++) {
            printf("\tLigne %d : %d -> %d\n", i, f->index[i], f->index[i+1]-1);
        }
        printf("\tLigne %d : %d -> EOF\n", i, f->index[i]);

        if(f->lines) {
            printf("Fichier de petite taille en mémoire :");

            for(i = 0; i < f->line_max; i++)
                printf("\n%s", f->lines[i]);

            putchar('\n');
        }


        if(p->show_c_function) {
            printf("Fonctions C :");

            for(i = 0; i+1 < f->c_func_nb; i++) {
                printf("\tFonction %d : %d -> %d\n", i, f->c_func[i], f->c_func[i+1]-1);
            }
            printf("\tFonction %d : %d -> EOF\n", i, f->c_func[i]);

        }
    }
}
