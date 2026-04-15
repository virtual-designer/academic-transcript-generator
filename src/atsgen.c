#include <stdio.h>
#include <string.h>

#define PROMPT ">>> "

static const int NAME_MAX = 128;

static const char *course_codes[] = {
    "CSE115",
    "CSE115L",
    "CSE173",
    "CSE215",
    "CSE215L",
    "CSE225",
    "CSE225L",
    "MAT116",
    "MAT120",
    "MAT125",
    "ENG103",
    "ENG105",
    "ENG111",
};


static int course_credits[] = {
    3,
    1,
    3,
    3,
    1,
    3,
    1,
    3,
    3,
    3,
    3,
    3,
    3,
};

static const char *course_names[] = {
    "Programming Language I",
    "Programming Language I Lab",
    "Discrete Mathematics",
    "Programming Language II",
    "Programming Language II Lab",
    "Data Structures & Algorithms",
    "Data Structures & Algorithms Lab",
    "Pre-calculus",
    "Calculus I",
    "Linear Algebra & Analytical Geometry",
    "Intermediate Composition",
    "Advanced Composition",
    "Public Speaking",
};

static const int course_count = 13;
static const int MAX_COURSES = 100;

static double calc_gp(int marks)
{
    if (marks >= 93)
        return 4.0;
    else if (marks >= 90)
        return 3.7;
    else if (marks >= 87)
        return 3.3;
    else if (marks >= 83)
        return 3.0;
    else if (marks >= 80)
        return 2.7;
    else if (marks >= 77)
        return 2.3;
    else if (marks >= 73)
        return 2.0;
    else if (marks >= 70)
        return 1.7;
    else if (marks >= 67)
        return 1.3;
    else if (marks >= 60)
        return 1.0;
    else
        return 0.0;
}

static const char *get_letter_grade(double gp)
{
    if (gp == 4.0)
        return "A";
    else if (gp >= 3.7)
        return "A-";
    else if (gp >= 3.3)
        return "B+";
    else if (gp >= 3.0)
        return "B";
    else if (gp >= 2.7)
        return "B-";
    else if (gp >= 2.3)
        return "C+";
    else if (gp >= 2.0)
        return "C";
    else if (gp >= 1.7)
        return "C-";
    else if (gp >= 1.3)
        return "D+";
    else if (gp >= 1.0)
        return "D";
    else
        return "F";
}

static void set_semester(int *semester)
{
    printf(PROMPT "Enter semester number (e.g. 261): ");
    scanf("%d", semester);
}

static void set_name(char *name)
{
    printf(PROMPT "Enter your name: ");
    fgets(name, NAME_MAX, stdin);
    name[strcspn(name, "\n")] = 0;
}

static void print_course_list(void)
{
    for (int i = 0; i < course_count; i++) {
        printf("[%d]: %s - %s\n", i + 1,
               course_codes[i], course_names[i]);
    }

    printf("\n");
}

static void add_courses_interactive(int *added_courses,
                                    int *added_course_marks,
                                    int *added_course_count)
{
    printf("Courses are listed below,");
    printf(" please enter the appropriate course IDs");
    printf(" indicated inside square brackets '[]').\n");
    printf("**Enter 0 to stop adding courses and finalize the transcript.**\n\n");

    print_course_list();

    for (;;) {
        printf(PROMPT "Enter course ID to add: ");

        int course_id = -1;
        scanf("%d", &course_id);

        if (course_id == 0)
            break;

        if (course_id < 1 || course_id > course_count) {
            printf("Invalid course ID: %d\n", course_id);
            continue;
        }

        course_id--;

        int marks = 0;
        printf(PROMPT "Enter marks for [%d] %s - %s: ",
               course_id + 1, course_codes[course_id],
               course_names[course_id]);

        scanf("%d", &marks);

        if (marks < 0 || marks > 100) {
            printf("Invalid marks: must be in between 0 to 100\n");
            continue;
        }

        added_courses[*added_course_count] = course_id;
        (*added_course_count)++;
        added_course_marks[course_id] = marks;

        printf("Course added: %s - %s [Marks: %d/100]\n\n", course_codes[course_id],
               course_names[course_id], marks);

        if ((*added_course_count) >= MAX_COURSES) {
            printf("Max course limit exceeded (%d), no more course entries allowed\n",
                   *added_course_count);
            break;
        }
    }
}

void print_transcript(const char *name, int semester, int *added_courses,
                      int *added_course_marks, int added_course_count)
{
    printf("Name:     %s\n", name);
    printf("Semester: %d\n", semester);
    printf("\n");
    printf("\nID\tCourse                                \tCredits  Marks  GP    Grade\n");

    double wgp = 0;
    double credits = 0;

    for (int i = 0; i < added_course_count; i++) {
        int course_id = added_courses[i];
        printf("[%d]\t%s - %s", course_id + 1, course_codes[course_id],
               course_names[course_id]);

        for (size_t j = 0; j < 38 - 3 - strlen(course_codes[course_id]) - strlen(course_names[course_id]); j++)
            printf(" ");

        double gp = calc_gp(added_course_marks[course_id]);

        printf("  %d        %-3d    %1.2lf  %s\n",course_credits[course_id],
                added_course_marks[course_id], gp,
                get_letter_grade(gp));

        credits += course_credits[course_id];
        wgp += gp * course_credits[course_id];
    }

    double sgpa = wgp / credits;
    printf("\nSGPA: %1.2lf (%s)\n", sgpa, get_letter_grade(sgpa));
}

int main(void)
{
    int added_courses[MAX_COURSES];
    int added_course_marks[MAX_COURSES];
    int added_course_count = 0;
    int semester = 0;
    char name[NAME_MAX];

    memset(name, 0, sizeof (name));
    memset(added_courses, 0, sizeof (added_courses));
    memset(added_course_marks, 0, sizeof (added_course_marks));

    printf("Starting interactive transcript generation.\n");
    set_name(name);
    set_semester(&semester);
    add_courses_interactive(added_courses, added_course_marks,
                            &added_course_count);
    printf("\n");
    print_transcript(name, semester, added_courses, added_course_marks,
                     added_course_count);
    return 0;
}
