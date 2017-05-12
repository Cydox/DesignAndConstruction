#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

	struct stringer* next;
} stringer;

stringer newStringer(double y, double w, double t) {
	stringer s = {y, {w, t, y, 1}, {t, w - t, y + t, 1}, 1};
	return s;
}

double stringerArea(const stringer* s) {
	return elementArea(&(s->e1)) + elementArea(&(s->e2));
}

double stringerQ(const stringer* s) {
	return elementQ(&(s->e1)) + elementQ(&(s->e2));
}

double stringerY(const stringer* s) {
	return stringerQ(s) / stringerArea(s);
}

double stringerIx(const stringer* s) {
	return elementIx(&(s->e1)) + elementArea(&(s->e1)) * (elementY(&(s->e1)) - stringerY(s)) * (elementY(&(s->e1)) - stringerY(s)) + elementIx(&(s->e2)) + elementArea(&(s->e2)) * (elementY(&(s->e2)) - stringerY(s)) * (elementY(&(s->e2)) - stringerY(s)) + stringerArea(s) * stringerY(s) * stringerY(s);
}



int main() {
	element e = {0.1f, 0.1f, 0.0f, .constant = 1};

	stringer s = newStringer(10.0f, 20.0f, 1.5f);
	
	printf("%f\n", stringerIx(&s));
	printf("%f\n", stringerY(&s));
	//printf("%f\n", elementArea(&e));
	//printf("%f\n", elementY(&e));
	//printf("%f\n", elementQ(&e));
	//printf("%f\n", elementIx(&e));
	return 0;
}
