#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
typedef struct {
    int year, month, day;
} Date;

typedef struct {
    Date dateOperation;
    Date dateValeur;
    double montant;
    double soldeApres;
    int nbJours;
    double interetPeriode;
} Operation;

typedef struct {
    double soldeInitial;
    double tauxAnnuel;          
    double tauxJournalier;      
    double pourcentageCap;      
    Operation* operations;
    int nbOperations;
} CompteEpargne;
Date parseDate(const char* str) {
    Date d;
    sscanf(str, "%d-%d-%d", &d.year, &d.month, &d.day);
    return d;
}

int comparerDate(Date a, Date b) {
    if (a.year != b.year) return a.year - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.day - b.day;
}

int dateDiff(Date d1, Date d2) {
    struct tm t1 = {0}, t2 = {0};
    t1.tm_year = d1.year - 1900; t1.tm_mon = d1.month - 1; t1.tm_mday = d1.day;
    t2.tm_year = d2.year - 1900; t2.tm_mon = d2.month - 1; t2.tm_mday = d2.day;
    return (int)((mktime(&t2) - mktime(&t1)) / 86400);
}

Date addDays(Date d, int days) {
    struct tm t = {0};
    t.tm_year = d.year - 1900; t.tm_mon = d.month - 1; t.tm_mday = d.day + days;
    mktime(&t);
    return (Date){ t.tm_year + 1900, t.tm_mon + 1, t.tm_mday };
}

int estFinTrimestre(Date d) {
    return (d.month == 3  && d.day == 31) ||
           (d.month == 6  && d.day == 30) ||
           (d.month == 9  && d.day == 30) ||
           (d.month == 12 && d.day == 31);
}
void ajouterOperation(CompteEpargne* c, Operation op) {
    c->operations = realloc(c->operations, (c->nbOperations + 1) * sizeof(Operation));
    c->operations[c->nbOperations++] = op;
}

void trierParDateOperation(CompteEpargne* c) {
	int i,j;
    for (i = 0; i < c->nbOperations - 1; i++)
        for (j = 0; j < c->nbOperations - i - 1; j++)
            if (comparerDate(c->operations[j].dateOperation,
                             c->operations[j + 1].dateOperation) > 0) {
                Operation tmp = c->operations[j];
                c->operations[j] = c->operations[j + 1];
                c->operations[j + 1] = tmp;
            }
}
void calculerCompte(CompteEpargne* c) {
    double solde = c->soldeInitial;
    double sommeInteretsTrimestre = 0.0;

    // dateDebutPeriode sert à calculer le nombre de jours depuis la dernière opération
    Date dateDebutPeriode = addDays(c->operations[0].dateOperation, 0);
	int i;
    for (i = 0; i < c->nbOperations; i++) {
        Operation* op = &c->operations[i];

        // date valeur = date opération + 7 jours
        op->dateValeur = addDays(op->dateOperation, 7);

        // nombre de jours depuis la dernière opération
        op->nbJours = dateDiff(dateDebutPeriode, op->dateValeur);
        if (op->nbJours < 0) op->nbJours = 0;

        // calcul intérêt période
        op->interetPeriode = solde * op->nbJours * c->tauxJournalier;

        // mise à jour solde avec le montant de l'opération
        solde += op->montant;
        op->soldeApres = solde;

        sommeInteretsTrimestre += op->interetPeriode;

        if (estFinTrimestre(op->dateValeur)) {
            solde += sommeInteretsTrimestre * (c->pourcentageCap / 100.0);
            sommeInteretsTrimestre = 0.0;
        }

        // mise à jour dateDebutPeriode pour la prochaine opération
        dateDebutPeriode = op->dateValeur;
    }
}
void genererHTML(CompteEpargne* c) {
    FILE* f = fopen("resultat.html", "w");
    if (!f) return;

    fprintf(f,
        "<!DOCTYPE html>"
        "<html><head><meta charset='UTF-8'>"
        "<title>Compte Épargne</title>"
        "<style>"
        "body{font-family:Arial;background:#f4f6f8;padding:20px;}"
        "h2{color:#2c3e50;}"
        "table{border-collapse:collapse;width:100%;background:white;}"
        "th,td{padding:10px;border:1px solid #ccc;text-align:center;}"
        "th{background:#34495e;color:white;}"
        "tr:nth-child(even){background:#f2f2f2;}"
        "</style>"
        "</head><body>");

    fprintf(f, "<h2>Simulation Compte d'Épargne</h2>");
    fprintf(f, "<p>Taux annuel : %.2f %%</p>", c->tauxAnnuel);
    fprintf(f, "<p>Capitalisation : %.0f %% des intérêts</p>", c->pourcentageCap);

    fprintf(f, "<table>"
               "<tr>"
               "<th>Date opération</th>"
               "<th>Date valeur</th>"
               "<th>Montant</th>"
               "<th>Solde</th>"
               "<th>Nb jours</th>"
               "<th>Intérêt période</th>"
               "</tr>");
	int i;
    for (i = 0; i < c->nbOperations; i++) {
        Operation op = c->operations[i];
        fprintf(f,
            "<tr>"
            "<td>%04d-%02d-%02d</td>"
            "<td>%04d-%02d-%02d</td>"
            "<td>%.2f</td>"
            "<td>%.2f</td>"
            "<td>%d</td>"
            "<td>%.4f</td>"
            "</tr>",
            op.dateOperation.year, op.dateOperation.month, op.dateOperation.day,
            op.dateValeur.year, op.dateValeur.month, op.dateValeur.day,
            op.montant, op.soldeApres, op.nbJours, op.interetPeriode
        );
    }

    fprintf(f, "</table></body></html>");
    fclose(f);
}
int main() {
    CompteEpargne c = {0};

    printf("Solde initial : ");
    scanf("%lf", &c.soldeInitial);

    printf("Taux d'intérêt annuel (%%) : ");
    scanf("%lf", &c.tauxAnnuel);

    printf("Pourcentage des intérêts capitalisés (ex: 80) : ");
    scanf("%lf", &c.pourcentageCap);

    c.tauxJournalier = c.tauxAnnuel / 100.0 / 360.0;
    getchar();

    char cont;
    do {
        char dateStr[20], type;
        double montant;

        printf("\nDate opération (YYYY-MM-DD) : ");
        fgets(dateStr, sizeof(dateStr), stdin);
        dateStr[strcspn(dateStr, "\n")] = 0;

        printf("Type (V = Versement / R = Retrait) : ");
        scanf(" %c", &type);

        printf("Montant : ");
        scanf("%lf", &montant);
        getchar();

        if (toupper(type) == 'R') montant = -montant;

        Operation op = {0};
        op.dateOperation = parseDate(dateStr);
        op.montant = montant;

        ajouterOperation(&c, op);

        printf("Ajouter une autre opération ? (O/N) : ");
        scanf(" %c", &cont);
        getchar();

    } while (toupper(cont) == 'O');

    trierParDateOperation(&c);
    calculerCompte(&c);
    genererHTML(&c);

    free(c.operations);

    printf("\n? Fichier resultat.html généré avec succès\n");
    return 0;
}
    
