// gcc main.c -Wall -o main.out && ./main.out < testuni
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

// tailles de tableaux
#define TAILLE 12
#define NB_DEPLACEMENTS 500

// definition des tableaux
typedef char t_Plateau[TAILLE][TAILLE];
typedef char typeDeplacements[NB_DEPLACEMENTS];

// definition des char a enregistrer / afficher
const char SOKOBAN[1] = "@";
const char CAISSES[1] = "$";
const char CIBLES[1] = ".";
const char MURS[1] = "#";
const char ESPACE[1] = " ";
const char CAISSES_SUR_CIBLES[1] = "*";
const char SOKOBAN_SUR_CIBLE[1] = "+";
const char SOK_GAUCHE = 'g';
const char CAISSE_GAUCHE = 'G';
const char SOK_DROITE = 'd';
const char CAISSE_DROITE = 'D';
const char SOK_HAUT = 'h';
const char CAISSE_HAUT = 'H';
const char SOK_BAS = 'b';
const char CAISSE_BAS = 'B';
//5

// temps entre chaque deplacements
const int DUREE_PAUSE = 200000;

// prototypes de toutes les fonctions / procedures
void lecture_niveau(char niveau[]);
int kbhit();
void charger_partie(t_Plateau plateau, char fichier[]);
void afficher_plateau(t_Plateau plateau, t_Plateau niveau);
void affiche_entete(char niveau[], int compteurDep);
void lecture_touches(char *Adr_touche);
void deplacer(typeDeplacements deplacement, t_Plateau plateau, int x, int y, int *compteur, bool possible, int *compteurDep);
void detection_sokoban(t_Plateau plateau, int *AdrX, int *AdrY);
bool gagne(t_Plateau plateau, t_Plateau niveau);
bool deplacement_possible(typeDeplacements deplacement, t_Plateau plateau, int x, int y, int compteur);
void chargerDeplacements(typeDeplacements t, char fichier[], int * nb);


int main(){
    //declaration des variables
    bool victoire = false, depPossible;
    t_Plateau plateau, niveau;
    char nomNiveau[30], nomDeplacement[30];
    int compteur, compteurDep, nbDep;
    int sokobanX, sokobanY;
    typeDeplacements deplacements;
    lecture_niveau(nomNiveau);
    charger_partie(niveau, nomNiveau);

    
    // remise a 0
    compteur = 0;
    compteurDep = 0;

    // initialisation
    charger_partie(plateau, nomNiveau);
    printf("Fichier deplacements (.dep) : ");
    scanf("%s",nomDeplacement);
    chargerDeplacements(deplacements, nomDeplacement, &nbDep);
    printf("nbDep : %d\n",nbDep);
    system("clear");
    affiche_entete(nomNiveau, compteurDep);
    afficher_plateau(plateau, niveau);

    while (compteur < nbDep){ 
        usleep(DUREE_PAUSE); // pause entre chaque mouvement
        detection_sokoban(plateau, &sokobanX, &sokobanY); // coordonnées de sokoban
        depPossible = false; // remets la verification de deplacement a false
        depPossible = deplacement_possible(deplacements, plateau, sokobanX, sokobanY, compteur); // verifie que le prochain deplacement est possible
        deplacer(deplacements, plateau, sokobanX, sokobanY, &compteur, depPossible, &compteurDep); // deplace sokoban
        system("clear");
        affiche_entete(nomNiveau, compteurDep);
        afficher_plateau(plateau, niveau);
    }
    victoire = gagne(plateau, niveau);
    if (victoire == true){ // si la partie est gagné
        printf("---------------------------------------------------------------------------------------------------------------\n");
        printf("La suite de déplacement \"%s\" est bien une solution pour la partie \"%s\".\n\n", nomDeplacement, nomNiveau);
        printf("Elle contient %d déplacements.\n", compteurDep);
        printf("---------------------------------------------------------------------------------------------------------------\n");
    }
    else{
        printf("---------------------------------------------------------------------------------------------------------------\n");
        printf("La suite de déplacements \"%s\" N’EST PAS une solution pour la partie \"%s\" .\n", nomDeplacement, nomNiveau);
        printf("---------------------------------------------------------------------------------------------------------------\n");
    }
    return EXIT_SUCCESS;
}
void lecture_niveau(char niveau[]){
    printf("nom du fichier .sok : ");
    scanf("%s", niveau);
}

void charger_partie(t_Plateau plateau, char fichier[]){
    FILE *f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f == NULL){
        printf("ERREUR SUR FICHIER\n");
        exit(EXIT_FAILURE);
    }
    else{
        for (int ligne = 0; ligne < TAILLE; ligne++){
            for (int colonne = 0; colonne < TAILLE; colonne++){
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

void afficher_plateau(t_Plateau plateau, t_Plateau niveau){
    char caseAffiche;
    caseAffiche = ESPACE[0];
    for (int x = 0; x < TAILLE; x++){
        for (int y = 0; y < TAILLE; y++){
            char casePlateau[1], caseNiveau[1];
            casePlateau[0] = plateau[x][y];
            caseNiveau[0] = niveau[x][y];
            if (casePlateau[0] == MURS[0]){
                caseAffiche = MURS[0];
            }
            else if (casePlateau[0] == CAISSES[0] || casePlateau[0] == CAISSES_SUR_CIBLES[0]){
                caseAffiche = CAISSES[0];
                plateau[x][y] = CAISSES[0];
            }
            else if (casePlateau[0] == SOKOBAN[0] || casePlateau[0] == SOKOBAN_SUR_CIBLE[0]){
                caseAffiche = SOKOBAN[0];
            }
            else if (caseNiveau[0] == CIBLES[0] || caseNiveau[0] == CAISSES_SUR_CIBLES[0] || caseNiveau[0] == SOKOBAN_SUR_CIBLE[0]){
                if (casePlateau[0] != SOKOBAN[0] && casePlateau[0] != CAISSES[0]){
                    caseAffiche = CIBLES[0];
                    plateau[x][y] = CIBLES[0];
                }
            }
            else{
                caseAffiche = ESPACE[0];
            }
            printf("%c", caseAffiche);
        }
        printf("\n");
    }
}

void affiche_entete(char niveau[], int compteurDep){
    printf("SOKOBAN niveau : %s\n\n", niveau);
    printf("Nombre de deplacements : %d \n\n\n", compteurDep);
}

bool deplacement_possible(typeDeplacements deplacement, t_Plateau plateau, int x, int y, int compteur){ // fonction qui verifie si le prochain deplacemet est possible
    bool possible = false;
    char dep = tolower(deplacement[compteur]); // on passe tout les deplacements en minuscule car on verifie juste si le mouvement est possible
    
    if (dep == SOK_HAUT && x > 0 && plateau[x - 1][y] != MURS[0]){
        if (!(plateau[x - 1][y] == CAISSES[0] &&
              (plateau[x - 2][y] == MURS[0] || plateau[x - 2][y] == CAISSES[0]))){ // si la prochaine case n'est pas une caisse avec un mur ou une caisse derriere
                possible = true; // alors le deplacement est possible
        }
    }
    else if (dep == SOK_GAUCHE && y > 0 && plateau[x][y - 1] != MURS[0]){
        if (!(plateau[x][y - 1] == CAISSES[0] &&
              (plateau[x][y - 2] == MURS[0] || plateau[x][y - 2] == CAISSES[0]))){
                possible = true;
        }
    }
    else if (dep == SOK_BAS && x < (TAILLE - 1) && plateau[x + 1][y] != MURS[0]){
        if (!(plateau[x + 1][y] == CAISSES[0] &&
              (plateau[x + 2][y] == MURS[0] || plateau[x + 2][y] == CAISSES[0]))){
                possible = true;
        }
    }
    else if (dep == SOK_DROITE && y < (TAILLE - 1) && plateau[x][y + 1] != MURS[0]){
        if (!(plateau[x][y + 1] == CAISSES[0] &&
              (plateau[x][y + 2] == MURS[0] || plateau[x][y + 2] == CAISSES[0]))){
                possible = true;
        }
    }
    return possible;
}



void deplacer(typeDeplacements deplacement, t_Plateau plateau, int x, int y, int *compteur, bool possible, int *compteurDep){
    int i = *compteur;
    if(deplacement[i] == SOK_BAS && plateau[x + 1][y] != CAISSES[0]){ // si le deplacement et un sokoban simple est qu'il n'y a pas de caisses derriere
       if(possible){ // si le deplacement a été validé par deplacement_possible()
            plateau[x + 1][y] = SOKOBAN[0]; // case d'apres = sokoban
            plateau[x][y] = ESPACE[0]; // case d'avant = espace
            (*compteurDep)++; // deplacements reels +1
       } 
    }
    else if(deplacement[i] == SOK_HAUT && plateau[x - 1][y] != CAISSES[0]){ 
        if(possible){
            plateau[x - 1][y] = SOKOBAN[0];
            plateau[x][y] = ESPACE[0];
            (*compteurDep)++;
        }
    }
    else if(deplacement[i] == SOK_DROITE && plateau[x][y + 1] != CAISSES[0]){
        if(possible){
            plateau[x][y + 1] = SOKOBAN[0];
            plateau[x][y] = ESPACE[0];
            (*compteurDep)++;
        }
    }
    else if(deplacement[i] == SOK_GAUCHE && plateau[x][y - 1] != CAISSES[0]){
        if(possible){
            plateau[x][y - 1] = SOKOBAN[0];
            plateau[x][y] = ESPACE[0];
            (*compteurDep)++;
        }
    }
    else if(deplacement[i] == CAISSE_BAS && plateau[x + 1][y] == CAISSES[0]){ // si le deplacement est un sokoban caisse et qu'il y a bien une caisse derriere
        if(possible){ // si le deplacement a été validé par deplacement_possible()
            plateau[x + 1][y] = SOKOBAN[0]; // case d'apres = sokoban
            plateau[x][y] = ESPACE[0]; // case d'avant = espace
            plateau[x + 2][y] = CAISSES[0]; // case 2 fois apres = caisse
            (*compteurDep)++;
        }
    }
    else if(deplacement[i] == CAISSE_HAUT && plateau[x - 1][y] == CAISSES[0]){
        if(possible){
            plateau[x - 1][y] = SOKOBAN[0];
            plateau[x][y] = ESPACE[0];
            plateau[x - 2][y] = CAISSES[0];
            (*compteurDep)++;
        }
    }
    else if(deplacement[i] == CAISSE_DROITE && plateau[x][y + 1] == CAISSES[0]){
        if(possible){
            plateau[x][y + 1] = SOKOBAN[0];
            plateau[x][y] = ESPACE[0];
            plateau[x][y + 2] = CAISSES[0];
            (*compteurDep)++;
        }
    }
    else if(deplacement[i] == CAISSE_GAUCHE && plateau[x][y - 1] == CAISSES[0]){
        if(possible){
            plateau[x][y - 1] = SOKOBAN[0];
            plateau[x][y] = ESPACE[0];
            plateau[x][y - 2] = CAISSES[0];
            (*compteurDep)++;
        }
    }
    (*compteur)++;
}


void detection_sokoban(t_Plateau plateau, int *AdrX, int *AdrY){
    int x, y;
    bool trouve = false;
    for (x = 0; x < TAILLE; x++){
        for (y = 0; y < TAILLE; y++){
            if (plateau[x][y] == SOKOBAN[0] || plateau[x][y] == SOKOBAN_SUR_CIBLE[0]){
                trouve = true;
                break;
            }
        }
        if (trouve){
            break;
        }
    }
    if (trouve){
        *AdrX = x;
        *AdrY = y;
    }
    else{
        printf("sokoban introuvable\n");
    }
}

bool gagne(t_Plateau plateau, t_Plateau niveau){
    bool victoire = true;
    for (int x = 0; x < TAILLE; x++){
        for (int y = 0; y < TAILLE; y++){
            if ((niveau[x][y] == CIBLES[0] || niveau[x][y] == SOKOBAN_SUR_CIBLE[0]) || niveau[x][y] == CAISSES_SUR_CIBLES[0]){
                if (plateau[x][y] != CAISSES[0]){
                    victoire = false;
                }
            }
        }
    }
    return victoire;
}

void chargerDeplacements(typeDeplacements t, char fichier[], int * nb){
    FILE * f;
    char dep;
    *nb = 0;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("FICHIER NON TROUVE\n");
    } else {
        fread(&dep, sizeof(char), 1, f);
        if (feof(f)){
            printf("FICHIER VIDE\n");
        } else {
            while (!feof(f)){
                t[*nb] = dep;
                (*nb)++;
                fread(&dep, sizeof(char), 1, f);
            }
        }
    }
    fclose(f);
}