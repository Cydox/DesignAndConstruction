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

	int constant;
} element;

double elementArea(const element* e) {
	return e->constant * e->x * e->y;
}

double elementY(const element* e) {
	return e->y_pos + (e->y / 2);
}

double elementQ(const element* e) {
	return e->constant * elementArea(e) * elementY(e);
}

double elementIx(const element* e) {
	return e->constant * (1.0f / 12.0f) * (e->x * e->y * e->y * e->y);
}

typedef struct stringer {
	double y_pos;
	
	element e1;
	element e2;

	int constant;
} stringer;

stringer newStringer(double y, double w, double t) {
	stringer s = {y, {w, t, y, 1}, {t, w - t, y + t, 1}, 1};
	return s;
}

double stringerArea(const stringer* s) {
	return s->constant * (elementArea(&(s->e1)) + elementArea(&(s->e2)));
}

double stringerQ(const stringer* s) {
	return s->constant * (elementQ(&(s->e1)) + elementQ(&(s->e2)));
}

double stringerY(const stringer* s) {
	return stringerQ(s) / stringerArea(s);
}

double stringerIx(const stringer* s) {
	return s->constant * (elementIx(&(s->e1)) + elementArea(&(s->e1)) * (elementY(&(s->e1)) - stringerY(s)) * (elementY(&(s->e1)) - stringerY(s)) + elementIx(&(s->e2)) + elementArea(&(s->e2)) * (elementY(&(s->e2)) - stringerY(s)) * (elementY(&(s->e2)) - stringerY(s))/* + stringerArea(s) * stringerY(s) * stringerY(s)*/);
}

typedef struct panel {
	element sheet;
	stringer stringer;
	int numberOfStringers;
} panel;

panel newPanel(int n, double sheetThickness, double stringerWidth, double stringerThickness, material m) {
	stringer stringer = newStringer(sheetThickness, stringerWidth, stringerThickness);
	element sheet = {PANEL_WIDTH, sheetThickness, 0, 1};
	panel p = {sheet, stringer, n};

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

int main() {
	element e = {0.1f, 0.1f, 0.0f, .constant = 1};

	stringer s = newStringer(0.0f, 20.0f, 1.5f);
	material al = {0,0,0,0};
	panel p = newPanel(3, 0.8f, 20.0f, 1.5f, al);

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
