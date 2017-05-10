#include <stdio.h>
#include <stdbool.h>

typedef struct element {
	float x;
	float y;

	//bottom left corner
	float x_pos;
	float y_pos;

	int constant;
} element;

float elementArea(const element* e) {
	return e->constant * e->x * e->y;
}

float elementY(const element* e) {
	return e->y_pos + (e->y / 2);
}

float elementQ(const element* e) {
	return e->constant * elementArea(e) * elementY(e);
}

float elementIx(const element* e) {
	return e->constant * (1.0f / 12.0f) * (e->x * e->y * e->y * e->y);
}

int main() {
	element e = {0.1f, 0.1f, 0.0f, 0.0f, .constant = 1};
	printf("%f\n", elementArea(&e));
	printf("%f\n", elementY(&e));
	printf("%f\n", elementQ(&e));
	printf("%f\n", elementIx(&e));
	return 0;
}
