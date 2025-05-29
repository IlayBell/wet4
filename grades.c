#include "grades.h"
#include <stdio.h>
#include <stdlib.h>
#include "linked-list.h"
#include <stdbool.h>
#include <string.h>

enum {
	FAIL = 1,
	SUCCESS = 0,
	AVG_FAIL = -1
};

/*
	linked list header
*/
typedef struct list* list;
typedef struct iterator* iterator;

/*
	structs
*/
struct course {
	char* name;
	int grade;
} typedef course;


struct student {
	char* name;
	int id;
	list courses;
} typedef student;


struct grades {
	list students;
} typedef grades;


/*
	student header
*/

student* student_init(const char* name, int id);
int student_clone(void* s, void** out);
void student_destroy(void* s);

bool student_id_comp(const void* s, const void* id);

struct list* copy_courses(student* s);

float student_calc_avg(student* s);

int add_course(student* s, course* c);

char* clone_name_student(student* s);

/*
	course header
*/
course* course_init(const char* name, int grade);
int course_clone(void* c, void** out);
void course_destroy(void* c);

bool course_name_comp(const void* c, const void* name);

/*
	common header
*/
iterator find(list l,
			  const void* target,
			  bool (*comp)(const void*, const void*));



/**
 * @brief Initializes the "grades" data-structure.
 * @returns A pointer to the data-structure, of NULL in case of an error
 */
grades* grades_init() {
	grades* g = (grades*)malloc(sizeof(grades));

	if (!g) {
		return NULL;
	}

	g->students = list_init(student_clone, student_destroy);

	return g;
}

/**
 * @brief Destroys "grades", de-allocate all memory!
 */
void grades_destroy(grades *grades) {
	list_destroy(grades->students);
	free(grades);
}

/**
 * @brief Adds a student with "name" and "id" to "grades"
 * @returns 0 on success
 * @note Failes if "grades" is invalid, or a student with 
 * the same "id" already exists in "grades"
 */
int grades_add_student(grades *grades, const char *name, int id) {
	if (!grades) {
		return FAIL;
	}

	if (find(grades->students, &id, student_id_comp)) {
		return FAIL;
	}

	student* s = student_init(name, id);

	int flag = list_push_back(grades->students, s);

	student_destroy(s);

	return flag;
}

/**
 * @brief Adds a course with "name" and "grade" to the student with "id"
 * @return 0 on success
 * @note Failes if "grades" is invalid, if a student with "id" does not exist
 * in "grades", if the student already has a course with "name", or if "grade"
 * is not between 0 to 100.
 */
int grades_add_grade(struct grades *grades,
                     const char *name,
                     int id,
                     int grade) {


	if (!grades || !(grade >= 0 && grade <= 100)) {
		return FAIL;
	}

	iterator student_iter = find(grades->students, &id, student_id_comp);
	if (!student_iter) {
		return FAIL;
	}

	student* s = (student*)list_get(student_iter);

	iterator course_iter = find(s->courses, name, course_name_comp);

	if (course_iter) {
		return FAIL;
	}

	course* new_course = course_init(name, grade);

	int flag = add_course(s, new_course);

	course_destroy(new_course);
	return flag;
}

/**
 * @brief Calcs the average of the student with "id" in "grades".
 * @param[out] out This method sets the variable pointed by "out" to the
 * student's name. Needs to allocate memory. The user is responsible for
 * freeing the memory.
 * @returns The average, or -1 on error
 * @note Fails if "grades" is invalid, or if a student with "id" does not exist
 * in "grades".
 * @note If the student has no courses, the average is 0.
 * @note On error, sets "out" to NULL.
 */
float grades_calc_avg(grades *grades, int id, char **out) {
	if (!grades) {
		out = NULL;
		return AVG_FAIL;
	}

	iterator student_iter = find(grades->students, &id, student_id_comp);
	if (!student_iter) {
		out = NULL;
		return AVG_FAIL;
	}

	student* s = (student*)list_get(student_iter);
	*out = clone_name_student(s);

	if (!list_size(s->courses)) {
		return 0;
	}

	return student_calc_avg(s);
}

/**
 * @brief Prints the courses of the student with "id" in the following format:
 * STUDENT-NAME STUDENT-ID: COURSE-1-NAME COURSE-1-GRADE, [...]
 * @returns 0 on success
 * @note Fails if "grades" is invalid, or if a student with "id" does not exist
 * in "grades".
 * @note The courses should be printed according to the order 
 * in which they were inserted into "grades"
 */
int grades_print_student(struct grades *grades, int id) {
	if (!grades) {
		return FAIL;
	}

	iterator student_iter = find(grades->students, &id, student_id_comp);
	if (!student_iter) {
		return FAIL;
	}

	student* s = list_get(student_iter);
	printf("%s %d: ", s->name, s->id);

	iterator course_iter = list_begin(s->courses);

	if (list_size(s->courses)) {
		while (list_next(course_iter)) {
			course* c = (course*)list_get(course_iter);
			printf("%s %d, ", c->name, c->grade);
			course_iter = list_next(course_iter);
		}
		course* c = (course*)list_get(course_iter);
		printf("%s %d", c->name, c->grade);
		course_iter = list_next(course_iter);
	}

	printf("\n");
	return SUCCESS;
}

/**
 * @brief Prints all students in "grade", in the following format:
 * STUDENT-1-NAME STUDENT-1-ID: COURSE-1-NAME COURSE-1-GRADE, [...]
 * STUDENT-2-NAME STUDENT-2-ID: COURSE-1-NAME COURSE-1-GRADE, [...]
 * @returns 0 on success
 * @note Fails if "grades" is invalid
 * @note The students should be printed according to the order 
 * in which they were inserted into "grades"
 * @note The courses should be printed according to the order 
 * in which they were inserted into "grades"
 */
int grades_print_all(struct grades *grades) {
	if (!grades) {
		return FAIL;
	}

	iterator student_iter = list_begin(grades->students);
	while (student_iter) {
		student* s = list_get(student_iter);
		grades_print_student(grades, s->id);

		student_iter = list_next(student_iter);
	}

	return SUCCESS;
}

/*
	student
*/

student* student_init(const char* name, int id) {
	student* s = (student*)malloc(sizeof(student));
	if (!s) {
		return NULL;
	}

	char* name_copy = (char*)malloc(sizeof(char)*(strlen(name)+1));
	if (!name_copy) {
		return NULL;
	}

	strcpy(name_copy, name);


	s->name = name_copy;
	s->id = id;

	list c = list_init(course_clone, course_destroy);
	s->courses = c;

	return s;
}

int student_clone(void* s, void** out) {
	if (!s || !out) {
		*out = NULL;
		return FAIL;
	}

	student* s_cast = (student*)s;
	student* s_copy = (student*)malloc(sizeof(student));
	if (!s_copy) {
		*out = NULL;
		return FAIL;
	}

	char* name_copy = (char*)malloc(sizeof(char)*(strlen(s_cast->name)+1));
	if (!name_copy) {
		*out = NULL;
		return FAIL;
	}

	strcpy(name_copy, s_cast->name);

	list c_copy = copy_courses(s_cast);
	
	if (!c_copy) {
		*out = NULL;
		return FAIL;
	}

	s_copy->name = name_copy;
	s_copy->id = s_cast->id;

	s_copy->courses = c_copy;

	*out = s_copy;

	return SUCCESS;
}

void student_destroy(void* s) {
	student* s_cast = (student*)s;
	free(s_cast->name);
	list_destroy(s_cast->courses);
	free(s_cast);
}

list copy_courses(student* s) {
	list copy = list_init(course_clone, course_destroy);

	iterator iter = list_begin(s->courses);

	while (iter != NULL) {
		if (list_push_back(copy, list_get(iter))) {
			return NULL;
		}

		iter = list_next(iter);
	}

	return copy;

}

bool student_id_comp(const void* s, const void* id) {
	student* s_cast = (student*)s;
	int* id_cast = (int*)id;
	return s_cast->id == *id_cast;
}

float student_calc_avg(student* s) {
	iterator iter = list_begin(s->courses);
	float sum = 0;
	while (iter != NULL) {
		course* c = list_get(iter);
		sum += c->grade;

		iter = list_next(iter);
	} 

	return sum / list_size(s->courses);
}

int add_course(student* s, course* c) {
	return list_push_back(s->courses, c);
}


char* clone_name_student(student* s) {
	char* name_copy = (char*)malloc(sizeof(char)*(strlen(s->name)+1));
	if(!name_copy) {
		return NULL;
	}
	strcpy(name_copy, s->name);

	return name_copy;
}

/*
	course
*/

course* course_init(const char* name, int grade) {
	course* c = (course*)malloc(sizeof(course));
	if (!c) {
		return NULL;
	}

	char* name_copy = (char*)malloc(sizeof(char)*(strlen(name)+1));
	if(!name_copy) {
		return NULL;
	}

	strcpy(name_copy, name);

	c->name = name_copy;
	c->grade = grade;

	return c;
}

int course_clone(void* c, void** out) {
	if(!c || !out) {
		*out = NULL;
		return FAIL;
	}

	course* c_cast = (course*)c;

	*out = course_init(c_cast->name, c_cast->grade);


	return SUCCESS;

}

void course_destroy(void* c) {
	course* c_cast = (course*)c;
	free(c_cast->name);
	free(c_cast);
}

bool course_name_comp(const void* c, const void* name) {
	course* c_cast = (course*)c;
	char* name_cast = (char*)name;

	return !strcmp(c_cast->name, name_cast);
}

/*
	common
*/

iterator find(list l,
			  const void* target,
			  bool (*comp)(const void*, const void*)) {
	iterator iter = list_begin(l);
	
	while (iter != NULL) {
		if (comp(list_get(iter), target)) {
			return iter;
		}

		iter = list_next(iter);
	}

	return NULL;
}
