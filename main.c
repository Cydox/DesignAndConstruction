#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define PANEL_WIDTH 400
#define PANEL_LENGTH 500


typedef struct material{
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
	double Kc[] = {3, 4, 5};
	
	double pitch = PANEL_WIDTH / (p->numberOfStringers - 1.0);

	return panelArea(p) * Kc[p->numberOfStringers] * p->m.E * (p->sheet.y / pitch) * (p->sheet.y / pitch);
}


int main() {
	//element e = {0.1, 0.1, 0.0, .constant = 1};

	stringer s = newStringer(0.0, 20.0, 1.5);
	material al = {0,0,0,0};
	panel p = newPanel(3, 0.8, 20.0, 1.5, al);

	printf("%f\n", panelY(&p));
	printf("%f\n", stringerIx(&(p.stringer)));
	printf("%f\n", stringerY(&(p.stringer)));
	printf("%f\n", stringerY(&s));
	printf("%f\n", panelIx(&p));
	//printf("%f\n", elementArea(&e));
	//printf("%f\n", elementY(&e));
	//printf("%f\n", elementQ(&e));
	//printf("%f\n", elementIx(&e));
	return 0;
}
