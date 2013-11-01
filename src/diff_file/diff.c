#include "diff.h"


/* Prototype des statiques */
static int diff_file_intern(t_diff **diff_list, t_index *f1, t_index *f2);

/* ===============================================
                      sec_fopen

    Ouverture sécurisée, retour le résultat de
    fopen et affiche l'erreur s'il y en a une.
    ----------------------------------------------
    c char *path : chemin
    c char *mode : mode d'ouverture
    ----------------------------------------------
    Retourne le résultat de fopen avec path et
    mode.
   =============================================== */
FILE* sec_fopen(const char* path, const char* mode) {
    FILE* f = fopen(path, mode);

    if(!f)
        perror(path);

    return f;
}

/* ===============================================
                      sec_fclose

    Fermeture sécurisée, affiche l'erreur s'il y
    en a une.
    ----------------------------------------------
    FILE *f : Flux à fermer
   =============================================== */
void sec_fclose(FILE *f) {

    if(fclose(f) != 0)
        perror("fclose()");
}

/* ===============================================
               diff_file_make_intern

    Fonctionnement interne du diff entre deux
    fichier. Fonction récursive qui s'apelle
    elle-même en interne. Met à jour diff_list.
    ----------------------------------------------
    t_diff **diff_list : Liste de modifications
    t_index *f1        : Index du fichier 1
    t_index *f2        : Index du fichier 2
    ----------------------------------------------
    Retourne le nombre minimum de mouvement de ligne
    pour passer de f1 à f2 à partir de la position
    courante de f1 et de f2 (f1->line et f2->line)
    et indique la liste d'opération dans diff_list.
   =============================================== */
static int diff_file_intern(t_diff **diff_list, t_index *f1, t_index *f2) {

    int line = 0;
    int val_supp = INT_MAX, val_add = INT_MAX, val_modif = INT_MAX; // Par défaut on force les valeurs à être au maximum
    t_diff *diff_supp = NULL, *diff_add_ = NULL, *diff_modif = NULL;

    unsigned int save = 0;

    unsigned int save_1 = 0;
    unsigned int save_2 = 0;

    /* On avance */
    line_error l1 = line_go_to(f1, f1->line+1);
    line_error l2 = line_go_to(f2, f2->line+1);


    /* Il reste des lignes */
    if(l1 == _NO_ERROR && l2 == _NO_ERROR ) {

        save_1 = f1->line;
        save_2 = f2->line;

        /* Test d'égalité */
        if(line_compare(f1, f2) != 0) {

            /* Test de la suppression */
            if((line = line_search(f2, f1)) >= 0) {

                /* Suppression de la ligne de f1 dans f2 possible */
                val_supp = ((unsigned)line) - f1->line;
                diff_add(&diff_supp, DELETED_LINE, f1->line, (unsigned)line-1, f2->line-1, f2->line-1);

                /* Déplacement */
                line_go_to(f1, (unsigned)(line)); //Possible sans test, car correspondance trouvée à line

                val_supp = val_supp + diff_file_intern(&diff_supp, f1, f2);
            }

            f1->line = save_1;
            f2->line = save_2;

            /* Test de l'ajout */
            if((line = line_search(f1, f2)) >= 0) {

                /* Ajout des lignes avant correspondance possible */
                val_add = ((unsigned)line) - f2->line;
                diff_add(&diff_add_, ADDED_LINE, f1->line-1, f1->line-1, f2->line, (unsigned)line-1);

                /* Déplacement */
                line_go_to(f2, (unsigned)(line)); //Possible sans test, car correspondance trouvée à line

                val_add = val_add + diff_file_intern(&diff_add_ ,f1, f2);
            }

            f1->line = save_1;
            f2->line = save_2;

            /* Test de la modification - Toujours possible */
            save = f1->line;

            do {
                if(line_next(f1) != _NO_ERROR)
                    break; // Si on ne peut plus avancer, on saute
            }while((line = line_search(f1, f2)) < 0);

            /* On a trouvé */
            if(line >= 0) {
                val_modif = (f1->line - save) + (((unsigned)(line)) - f2->line);
                diff_add(&diff_modif, CHANGED_LINE, save, f1->line-1, f2->line, (unsigned)(line-1));

                /* On remonte de la ligne dépassée */
                line_previous(f1);
            }
            /* Pas de correspondance jusqu'à fin du fichier */
            else {
                val_modif = ((f1->line_max) - save) + ((f2->line_max) - f2->line);
                diff_add(&diff_modif, CHANGED_LINE, save, f1->line_max-1, f2->line, f2->line_max-1);

                line_go_to(f2, f2->line_max-1); // On va à la fin
                line_go_to(f1, f1->line_max-1);
            }

            val_modif = val_modif + diff_file_intern(&diff_modif ,f1, f2);


            /* On sélectionne le test ayant causé le moins de mouvement */
            if(val_supp <= val_add && val_supp <= val_modif) {
                /* Suppression */

                diff_delete(diff_add_);
                diff_delete(diff_modif);

                /* Ajout */
                if(*diff_list)
                    diff_last(*diff_list)->next = diff_supp;
                else
                    (*diff_list) = diff_supp;

                return val_supp;

            } else if(val_add <= val_supp && val_add <= val_modif) {
                /* Ajout */

                diff_delete(diff_supp);
                diff_delete(diff_modif);

                /* Ajout */
                if(*diff_list)
                    diff_last(*diff_list)->next = diff_add_;
                else
                    (*diff_list) = diff_add_;

                return val_add;

            } else {
                /* Modification */

                diff_delete(diff_supp);
                diff_delete(diff_add_);

                /* Ajout */
                if(*diff_list)
                    diff_last(*diff_list)->next = diff_modif;
                else
                    (*diff_list) = diff_modif;

                return val_modif;
            }
        }
        /* Lignes égales */
        else
            return diff_file_intern(diff_list ,f1, f2);

    }
    /* Fin du fichier d'un côté */
    else if (l1 != _NO_ERROR && l2 == _NO_ERROR){
        diff_add(diff_list, ADDED_LINE, f1->line_max-1, f1->line_max-1, f2->line, f2->line_max-1);
        return ((f2->line_max-1) - (f2->line-1));

    } else if (l1 == _NO_ERROR && l2 != _NO_ERROR){
        diff_add(diff_list, DELETED_LINE, f1->line, f1->line_max-1, f2->line_max-1, f2->line_max-1);
        return ((f1->line_max-1) - (f1->line-1));

    }
    /* Fin du fichier des deux côtés */
    else
        return 0;

}


int diff_file(const char* f1_name, const char* f2_name) {

    int ret = 0;

    FILE* f1 = sec_fopen(f1_name, "r+"), *f2 = sec_fopen(f2_name, "r+");

    t_index *i1 = NULL, *i2 = NULL;

    t_diff *diff_list = NULL;

    if(f1) {
        if(f2) {
            i1 = index_file(f1, f1_name);
            i2 = index_file(f2, f2_name);

            /* Si on doit afficher une fonction C */
            if(p->show_c_function)
                index_file_c_func(i1);

            i1->line = -1;
            i2->line = -1;

            /* Fichiers identiques */
            if(diff_file_intern(&diff_list, i1, i2) > 0) {
                ret = 1;
                if(p->brief) // Si pas d'affichage
                    printf("Files %s and %s differ\n", f1_name, f2_name);
                else
                    diff_display(diff_list, i1 ,i2);
            }
            /* Fichiers différents */
            else {
                ret = 0;

                if(p->o_style == COLUMNS) // Même si le fichier est identique, il faut l'afficher
                    diff_display(diff_list, i1, i2);

                if(p->report_identical_files) // Doit s'ajouter à l'affichage par colonnes
                    printf("Files %s and %s are identical\n", f1_name, f2_name);
            }

            index_free(i1);
            index_free(i2);

            sec_fclose(f1);
            sec_fclose(f2);

        } else {
            sec_fclose(f1);
            ret = 2;

        }
    } else
        ret = 2;

    return ret;
}
