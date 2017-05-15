#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define PI 3.141592653589

#define PANEL_WIDTH 400
#define PANEL_LENGTH 500
#define C 4
#define RIVET_C 2.1
#define RIVET_SAFETY 0.8

#define BUCKLING_REQUIREMENT 15000
#define FAILURE_REQUIREMENT 30000


typedef struct material{
	char* name;
	double E;
	double rho;
	double sigmaYield;
	double sigmaUlt;
} material;

typedef struct element {
	double x;
	double y;

	double y_pos;

	bool notFailed;
} element;

double elementArea(const element* e) {
	if (!e->notFailed) {
		return 0;
	} else {
		return e->x * e->y;
	}
}

double elementY(const element* e) {
	return e->y_pos + (e->y / 2);
}

double elementQ(const element* e) {
	if (!e->notFailed) {
		return 0;
	} else {
		return elementArea(e) * elementY(e);
	}
}

double elementIx(const element* e) {
	if (!e->notFailed) {
		return 0;
	} else {
		return (1.0 / 12.0) * (e->x * e->y * e->y * e->y);
	}
}

typedef struct stringer {
	double y_pos;
	
	element e1;
	element e2;

	bool notFailed;
} stringer;

stringer newStringer(double y, double w, double t) {
	stringer s = {y, {w, t, y, true}, {t, w - t, y + t, true}, true};
	return s;
}

double stringerArea(const stringer* s) {
	if (!s->notFailed) {
		return 0;
	} else {
		return elementArea(&(s->e1)) + elementArea(&(s->e2));
	}
}

double stringerQ(const stringer* s) {
	if (!s->notFailed) {
		return 0;
	} else {
		return elementQ(&(s->e1)) + elementQ(&(s->e2));
	}
}

double stringerY(const stringer* s) {
	if (!s->notFailed) {
		return 0;
	} else {
		return stringerQ(s) / stringerArea(s);
	}
}

double stringerIx(const stringer* s) {
	if (!s->notFailed) {
		return 0;
	} else {
		return elementIx(&(s->e1)) + elementArea(&(s->e1)) * (elementY(&(s->e1)) - stringerY(s)) * (elementY(&(s->e1)) - stringerY(s)) + elementIx(&(s->e2)) + elementArea(&(s->e2)) * (elementY(&(s->e2)) - stringerY(s)) * (elementY(&(s->e2)) - stringerY(s))/* + stringerArea(s) * stringerY(s) * stringerY(s)*/;
	}
}

typedef struct panel {
	element sheet;
	stringer stringer;
	material m;
	int numberOfStringers;
} panel;

panel newPanel(int n, double sheetThickness, double stringerWidth, double stringerThickness, material m) {
	stringer stringer = newStringer(sheetThickness, stringerWidth, stringerThickness);
	element sheet = {PANEL_WIDTH, sheetThickness, 0, 1};
	panel p = {sheet, stringer, m, n};

	return p;
}

double panelArea(const panel* p) {
	return elementArea(&(p->sheet)) + p->numberOfStringers * stringerArea(&(p->stringer));
}

double panelQ(const panel* p) {
	return elementQ(&(p->sheet)) + p->numberOfStringers * stringerQ(&(p->stringer));
}

double panelY(const panel* p) {
	return panelQ(p) / panelArea(p);
}

double panelIx(const panel* p) {
	return elementIx(&(p->sheet)) + elementArea(&(p->sheet)) * (elementY(&(p->sheet)) - panelY(p)) * (elementY(&(p->sheet)) - panelY(p)) + p->numberOfStringers * (stringerIx(&(p->stringer)) + stringerArea(&(p->stringer)) * (stringerY(&(p->stringer)) - panelY(p)) * (stringerY(&(p->stringer)) - panelY(p)));
}

double panelYield(const panel* p) {
	return panelArea(p) * p->m.sigmaYield;
}

double panelUltFailure(const panel* p) {
	return panelArea(p) * p->m.sigmaUlt;
}

double panelSheetBuckling(const panel* p) {
	if (!p->sheet.notFailed) {
		return -1;
	} else {
		double Kc[] = {3.6,3.6,5.12,4,3.6,3.65,3.6,3.6,3.6,3.6};
		
		double pitch = PANEL_WIDTH / (p->numberOfStringers - 1.0);
		
		return panelArea(p) * Kc[p->numberOfStringers] * p->m.E * (p->sheet.y / pitch) * (p->sheet.y / pitch);
	}
}

double panelColumnBuckling(const panel* p) {
	if (!p->stringer.notFailed) {
		return -1;
	} else {
		return (C * PI * PI * p->m.E * panelIx(p)) / (PANEL_LENGTH * PANEL_LENGTH);
	}
}

int getIndexOfMinFail(double failures[3]) {
	int index = 0;
	double val = failures[0];

	for (int i = 1; i < 3; i++) {
		if (failures[i] < val && failures[i] != -1) {
			val = failures[i];
			index = i;
		}
	}
	return index;
}

double calcRivetSpace(const panel* p) {
	return RIVET_SAFETY * sqrt((0.9 * RIVET_C * p->m.E * p->sheet.y * p->sheet.y) / (BUCKLING_REQUIREMENT / panelArea(p)));
}

int numberOfRivets(const panel* p) {
	double s = calcRivetSpace(p);
	return (int) ((PANEL_LENGTH - (2 * 30)) / s) * p->numberOfStringers;
}

double interRivetBuckling(const panel* p) {
	if (!p->sheet.notFailed) {
		return -1;
	} else {
		double s = calcRivetSpace(p);
		return 0.9 * RIVET_C * p->m.E * (p->sheet.y / s) * (p->sheet.y / s) * panelArea(p);
	}
}

double panelMass(const panel* p) {
	return (PANEL_LENGTH * panelArea(p) * p->m.rho / 1000) + ((numberOfRivets(p) * 0.488) - (p->m.rho * PI * 1.5 * 1.5 * (p->sheet.y + p->stringer.e2.x) / 1000));
}

void progressiveFailureAnalysis(material m, double sheetThickness, double stringerDimension[2]) {

	int numberOfStringers = 0;

	double failure = 0;
	double buckling = 0;

	for (int i = 2; (failure < FAILURE_REQUIREMENT) || (buckling < BUCKLING_REQUIREMENT); i++) {
		bool failed = false;
		bool buckled = false;

		panel p = newPanel(i, sheetThickness, stringerDimension[0], stringerDimension[1], m);

		printf("%s,%f,%f,%f,%f,%f,%d", m.name, panelMass(&p), sheetThickness, stringerDimension[0], stringerDimension[1], calcRivetSpace(&p), i);
		while (!failed) {
			double failures[] = {panelUltFailure(&p), panelSheetBuckling(&p), panelColumnBuckling(&p), interRivetBuckling(&p)};
			int index = getIndexOfMinFail(failures);
			
			if (index == 0) {
				failed = true;
				failure = failures[0];

				printf(",all,%f", failure);
			} else if (index == 1 && !buckled) {
				buckled = true;
				buckling = failures[1];
				p.sheet.notFailed = false;
				printf(",sheet,%f", buckling);
			} else if (index == 1 && buckled) {
				failed = true;
				failure = failures[1];
				printf(",sheet,%f", failure);
			} else if (index == 2 && !buckled) {
				buckled = true;
				buckling = failures[2];
				p.stringer.notFailed = false;
				printf(",stringers,%f", buckling);
			} else if (index == 2 && buckled) {
				failed = true;
				failure = failures[2];
				printf(",stringers,%f", failure);
			} else if (index == 3 && !buckled) {
				buckled = true;
				buckling = failures[3];
				printf(",interrivet,%f", buckling);
			} else if (index == 3 && buckled) {
				failed = true;
				failure = failures[3];
				printf(",interrivet,%f", failure);
			}
		}

		printf("\n");
	}
}

int main() {
	/*//element e = {0.1, 0.1, 0.0, .constant = 1};

	stringer s = newStringer(0.0, 20.0, 1.5);
	material al = {72400,0,345,483};
	panel p = newPanel(5, 0.8, 20.0, 1.5, al);

	printf("%f\n", panelIx(&p));
	printf("%f\n", panelSheetBuckling(&p));
	printf("%f\n", panelColumnBuckling(&p));
	printf("%f\n", panelYield(&p));
	printf("%f\n", panelUltFailure(&p));
	//printf("%f\n", elementArea(&e));
	//printf("%f\n", elementY(&e));
	//printf("%f\n", elementQ(&e));
	//printf("%f\n", elementIx(&e));*/
	
	material aluminum = {"Aluminum", 72400, 2.78, 345, 483};
	material steel = {"Steel", 210000, 7.8, 1100, 1275};

	double sheetThicknesses[] = {0.8, 1.0, 1.2};

	double aluminumStringerDimensions[][2] = {{20, 1.5}, {20, 2}, {15, 1}, {15, 1.5}};
	double steelStringerDimensions[][2] = {{15, 1.5}, {15, 2}};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			progressiveFailureAnalysis(aluminum, sheetThicknesses[i], aluminumStringerDimensions[j]);
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			
			progressiveFailureAnalysis(steel, sheetThicknesses[i], steelStringerDimensions[j]);
		}
	}


	return 0;
}
