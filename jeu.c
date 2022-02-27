/*
	Canvas pour algorithmes de jeux à 2 joueurs
	
	joueur 0 : humain
	joueur 1 : ordinateur
			
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Paramètres du jeu
#define LARGEUR_MAX 7		// nb max de fils pour un noeud (= nb max de coups possibles)
#define TEMPS 5		// temps de calcul pour un coup avec MCTS (en secondes)
#define LARGEUR 7
#define HAUTEUR 6

// macros
#define AUTRE_JOUEUR(i) (1-(i))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))

// Critères de fin de partie
typedef enum {NON, MATCHNUL, ORDI_GAGNE, HUMAIN_GAGNE } FinDePartie;

// Definition du type Etat (état/position du jeu)
typedef struct EtatSt {

	int joueur; // à qui de jouer ?
	char plateau[HAUTEUR][LARGEUR];	

} Etat;

// Definition du type Coup
typedef struct {	
	int ligne;
	int colonne;

} Coup;

// Copier un état 
Etat * copieEtat( Etat * src ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));

	etat->joueur = src->joueur;
	
		
	// TODO: à compléter avec la copie de l'état src dans etat
	
	/* par exemple : */
	int i,j;	
	for (i=0; i< HAUTEUR; i++)
		for ( j=0; j<LARGEUR; j++)
			etat->plateau[i][j] = src->plateau[i][j];
	

	
	return etat;
}

// Etat initial 
Etat * etat_initial( void ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));
	int i,j;	
	for (i=0; i< HAUTEUR; i++)
		for ( j=0; j<LARGEUR; j++)
			etat->plateau[i][j] = ' ';
	
	return etat;
}


void afficheJeu(Etat * etat) {
	
	fflush(stdout);
	int i,j;
	printf("   |");
	for ( j = 0; j < LARGEUR; j++) 
		printf(" %d |", j);
	printf("\n");
	printf("--------------------------------");
	printf("\n");
	
	for(i=0; i < HAUTEUR; i++) {
		printf(" %d |", i);
		for ( j = 0; j < LARGEUR; j++) 
			printf(" %c |", etat->plateau[i][j]);
		printf("\n");
		printf("--------------------------------");
		printf("\n");
	}
}


// Nouveau coup 
// TODO: adapter la liste de paramètres au jeu
Coup * nouveauCoup( int i, int j ) {
	Coup * coup = (Coup *)malloc(sizeof(Coup));
	coup->ligne = i;
	coup->colonne = j;
	
	return coup;
}

// Demander à l'humain quel coup jouer 
Coup * demanderCoup (Etat *etat) {
	int i , j;
	printf("Coup prochain (sélectionnez un colonne de 0 à 6):");
	scanf("%d",&j);
	for(i=HAUTEUR-1;i>=0;i--){
		if(etat->plateau[i][j]==' '){
			break;
		}
	}
	return nouveauCoup(i,j);

}

// Modifier l'état en jouant un coup 
// retourne 0 si le coup n'est pas possible
int jouerCoup( Etat * etat, Coup * coup ) {

	if ( etat->plateau[coup->ligne][coup->colonne] != ' ' )
		return 0;
	else {
		etat->plateau[coup->ligne][coup->colonne] = etat->joueur ? 'O' : 'X';
		
		// à l'autre joueur de jouer
		etat->joueur = AUTRE_JOUEUR(etat->joueur); 	
		return 1;
	}	
}

// Retourne une liste de coups possibles à partir d'un etat 
// (tableau de pointeurs de coups se terminant par NULL)
Coup ** coups_possibles( Etat * etat ) {
	Coup ** coups = (Coup **) malloc((1+LARGEUR_MAX) * sizeof(Coup *) );	
	int k = 0;
	
	// TODO: à compléter
	
	/* par exemple */
	int i,j;
	for (j=0; j < LARGEUR; j++) {
		for(i=HAUTEUR-1;i>=0;i--){
			if ( etat->plateau[i][j] == ' ' ) {
				coups[k] = nouveauCoup(i,j); 
				k++;
				break;
			}
			
		}
	}
	
	/* fin de l'exemple */
	
	coups[k] = NULL;
	return coups;

}
// Definition du type Noeud 
typedef struct NoeudSt {
		
	int joueur; // joueur qui a joué pour arriver ici
	Coup * coup;   // coup joué par ce joueur pour arriver ici
	
	Etat * etat; // etat du jeu
			
	struct NoeudSt * parent; 
	struct NoeudSt * enfants[LARGEUR_MAX]; // liste d'enfants : chaque enfant correspond à un coup possible
	int nb_enfants;	// nb d'enfants présents dans la liste
	
	// POUR MCTS:
	int nb_victoires;
	int nb_simus;
	
} Noeud;


// Créer un nouveau noeud en jouant un coup à partir d'un parent 
// utiliser nouveauNoeud(NULL, NULL) pour créer la racine
Noeud * nouveauNoeud (Noeud * parent, Coup * coup ) {
	Noeud * noeud = (Noeud *)malloc(sizeof(Noeud));
	
	if ( parent != NULL && coup != NULL ) {
		noeud->etat = copieEtat ( parent->etat );
		jouerCoup ( noeud->etat, coup );
		noeud->coup = coup;			
		noeud->joueur = AUTRE_JOUEUR(parent->joueur);		
	}
	else {
		noeud->etat = NULL;
		noeud->coup = NULL;
		noeud->joueur = 0; 
	}
	noeud->parent = parent; 
	noeud->nb_enfants = 0; 
	
	// POUR MCTS:
	noeud->nb_victoires = 0;
	noeud->nb_simus = 0;	
	

	return noeud; 	
}

// Ajouter un enfant à un parent en jouant un coup
// retourne le pointeur sur l'enfant ajouté
Noeud * ajouterEnfant(Noeud * parent, Coup * coup) {
	Noeud * enfant = nouveauNoeud (parent, coup ) ;
	parent->enfants[parent->nb_enfants] = enfant;
	parent->nb_enfants++;
	return enfant;
}

void freeNoeud ( Noeud * noeud) {
	if ( noeud->etat != NULL)
		free (noeud->etat);
		
	while ( noeud->nb_enfants > 0 ) {
		freeNoeud(noeud->enfants[noeud->nb_enfants-1]);
		noeud->nb_enfants --;
	}
	if ( noeud->coup != NULL)
		free(noeud->coup); 

	free(noeud);
}
	

// Test si l'état est un état terminal 
// et retourne NON, MATCHNUL, ORDI_GAGNE ou HUMAIN_GAGNE
FinDePartie testFin( Etat * etat ) {

	// TODO...
	
	/* par exemple	*/
	
	// tester si un joueur a gagné
	int i,j,k,n = 0;
	for ( i=0;i < HAUTEUR; i++) {
		for(j=0; j < LARGEUR; j++) {
			if ( etat->plateau[i][j] != ' ') {
				n++;	// nb coups joués
			
				// colonnes
				k=0;
				while ( k < 4 && i-k < HAUTEUR && etat->plateau[i-k][j] == etat->plateau[i][j] ) 
					k++;
				if ( k == 4 ) 
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;

				// lignes
				k=0;
				while ( k < 4 && j+k < LARGEUR && etat->plateau[i][j+k] == etat->plateau[i][j] ) 
					k++;
				if ( k == 4 ) 
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;

				// diagonales : \
				k=0;
				while ( k < 4 && i+k < HAUTEUR && j+k < LARGEUR && etat->plateau[i+k][j+k] == etat->plateau[i][j] ) 
					k++;
				if ( k == 4 ) 
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;
				// diagonales : /
				k=0;
				while ( k < 4 && j+k < LARGEUR && i-k >= 0 && etat->plateau[i-k][j+k] == etat->plateau[i][j] ) 
					k++;
				if ( k == 4 ) 
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;		
			}
		}
	}

	// et sinon tester le match nul	
	if ( n == HAUTEUR*LARGEUR ) 
		return MATCHNUL;
		
	return NON;
}



// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes
void ordijoue_mcts(Etat * etat, int tempsmax) {
	clock_t tic, toc;
	tic = clock();
	int temps;
    //srand(time(NULL));
	Coup ** coups;
	Coup * meilleur_coup ;
	
	// Créer l'arbre de recherche
	Noeud * racine = nouveauNoeud(NULL, NULL);	
	racine->etat = copieEtat(etat); 
	
	// créer les premiers noeuds:
	coups = coups_possibles(racine->etat); 
	int k = 0;
	Noeud * enfant;
	while ( coups[k] != NULL) {
		enfant = ajouterEnfant(racine, coups[k]);
		k++;
	}
	
	//meilleur_coup = coups[ rand()%k ]; // choix aléatoire
	int iter = 0;	

    do {
        // Selectioner le noeud avec la plus grande B-Valeur recurcivement jusqu'a un fils sans B-valeur
        float maxBvaleur = 0;
        Noeud *noeudMaxBvaleur;
        bool trouve = false;
        Noeud *noeudCourant = racine;
        Noeud *enfantsTrouve[LARGEUR_MAX];
        int nbEnfantsTrouve = 0;

		int a = 1;
		// tant que on n'a pas trouvé de noeud a exploré on cherche recursivement
        while(!trouve){
			a++;
			maxBvaleur = INT_MIN;
            // si le noeud actuel n'est pas final
            if (testFin(noeudCourant->etat) == NON) {
                // on parcours ses enfants
                for (int i = 0; i < noeudCourant->nb_enfants; ++i) {
                    Noeud *noeudEnfantCourant = noeudCourant->enfants[i];
                    // si il existe un enfant sans simulation on l'ajoute a la liste des enfants a simulé
                    // et trouvé = true
                    if (noeudEnfantCourant->nb_simus == 0) {
                        trouve = true;
                        enfantsTrouve[nbEnfantsTrouve] = noeudEnfantCourant;
                        nbEnfantsTrouve++;
                    } else {
                        // sinon on calcule la bvaleur de tout les fils et on sauvegarde le meilleur noeud
                        float muI = (float) noeudEnfantCourant->nb_victoires / (float) noeudEnfantCourant->nb_simus;
                        if (noeudEnfantCourant->joueur == 0){
                            muI = -muI;
                        }
                        float Bvaleur =
                                muI + sqrt(2) * sqrt(log(noeudCourant->nb_simus) / noeudEnfantCourant->nb_simus);
						if (Bvaleur > maxBvaleur) {
							maxBvaleur = Bvaleur;
                            noeudMaxBvaleur = noeudEnfantCourant;
                        }
                    }
                }
                // si on a pas trouvé de fils sans simulation on prend celui avec la meilleur Bvaleur
                // on cree alors tout ses fils si il n'en a pas
                if (!trouve) {
                    noeudCourant = noeudMaxBvaleur;
                    if (noeudCourant->nb_enfants == 0) {
                        coups = coups_possibles(noeudCourant->etat);
                        k = 0;
                        while (coups[k] != NULL) {
                            ajouterEnfant(noeudCourant, coups[k]);
                            k++;
                        }
                    }
                }
            }else{
                enfantsTrouve[nbEnfantsTrouve] = noeudCourant;
                nbEnfantsTrouve++;
                trouve = true;
            }
        }

        // avec les fils sans B-Valeur => en choisir un aleatoirement
        Noeud *noeudChoisi = enfantsTrouve[rand() % nbEnfantsTrouve];

        //simuler une partie aléatoire
        Etat *etatDepart = duplicateState(noeudChoisi->etat);

        // on ne prend pas un coup gagnant si il y en a un
		if (amelioration == AMELIORE) {
			while(testFin(etatDepart) == NON){
				Coup **coupsDePartie = coups_possibles(etatDepart);
				int nbCoups = 0;
				while (coupsDePartie[nbCoups] != NULL) {
					nbCoups++;
				}
				Coup* coupChoisi = coupsDePartie[rand() % nbCoups];
				jouerCoup(etatDepart, coupChoisi);
			}
		
	
		} else { //avec amélioration
			Etat *etatFuture = duplicateState(etatDepart);
			bool etatFuturEstFinal = false;
			while(testFin(etatDepart) == NON){
				etatFuturEstFinal = false;
				Coup **coupsDePartie = coups_possibles(etatDepart);
				int nbCoups = 0;
				while (coupsDePartie[nbCoups] != NULL) {
					nbCoups++;
				}
				//simulation de coup suivant pour choisir forcement le coup gagnant si il existe
				for (int i = 0; i < nbCoups; i++) {
					etatFuture = duplicateState(etatDepart);
					jouerCoup(etatFuture, coupsDePartie[i]);
					if (testFin(etatFuture) == DEFAITE){
						etatFuturEstFinal = true;
						jouerCoup(etatDepart, coupsDePartie[i]);
						break;
					}
				}
				if (!etatFuturEstFinal){
					Coup* coupChoisi = coupsDePartie[rand() % nbCoups];
					jouerCoup(etatDepart, coupChoisi);
				}
			}
		}

        // remonter la valeur sur tout les neuds parcouru jusqu'a la racine
        FinDePartie resultat = testFin(etatDepart);
        noeudCourant = noeudChoisi;
        while(noeudCourant != NULL){
            if (resultat == DEFAITE){
                noeudCourant->nb_victoires += 1;
            }
            if (resultat == NUL){
                noeudCourant->nb_victoires += 0.5f;
            }
            noeudCourant->nb_simus++;
            noeudCourant = noeudCourant->parent;
        }

        free(etatDepart);
	

        toc = clock();
        temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
        iter ++;
    } while (temps < tempsmax);

    //fin de l'algorithme
    int maxN = 0;
    for (int j = 0; j < racine->nb_enfants; j++) {
		int nb;
		if(strategie == STRATEGIE) nb = racine->enfants[j]->nb_simus;
		else nb = racine->enfants[j]->nb_victoires;
        if (nb > maxN){
            meilleur_coup = racine->enfants[j]->coup;
            maxN = racine->enfants[j]->nb_simus;
        }
    }

    // Jouer le meilleur premier coup
    jouerCoup(etat, meilleur_coup);
	
	// Penser à libérer la mémoire :
	freeNoeud(racine);
	free (coups);
}

int main(void) {

	Coup * coup;
	FinDePartie fin;
	
	// initialisation
	Etat * etat = etat_initial(); 
	
	// Choisir qui commence : 
	printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
	scanf("%d", &(etat->joueur) );
	
	// boucle de jeu
	do {
		printf("\n");
		afficheJeu(etat);
		
		if ( etat->joueur == 0 ) {
			// tour de l'humain
			
			do {
				coup = demanderCoup(etat);

			} while ( !jouerCoup(etat, coup) );
									
		}
		else {
			// tour de l'Ordinateur
			ordijoue_mcts( etat, TEMPS );
			
		}
		
		fin = testFin( etat );
	}	while ( fin == NON ) ;

	printf("\n");
	afficheJeu(etat);
		
	if ( fin == ORDI_GAGNE )
		printf( "** L'ordinateur a gagné **\n");
	else if ( fin == MATCHNUL )
		printf(" Match nul !  \n");
	else
		printf( "** BRAVO, l'ordinateur a perdu  **\n");
	return 0;
}
