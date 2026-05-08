#include <stdio.h>
#include <string.h>

#define MAX_COURSES 100
#define COURSE_NAME_MAX 128
#define SEMESTER_COUNT_MAX 64

struct semester_record
{
    int semester;
    int course_ids[MAX_COURSES];
    int course_marks[MAX_COURSES];
    int course_ignored_flags[MAX_COURSES];
    int course_count;
};

struct semester_record_list
{
    struct semester_record list[SEMESTER_COUNT_MAX];
    int count;
};

char *course_codes[] = {
    "CSE115", "CSE115L", "CSE173", "CSE215", "CSE215L", "CSE225", "CSE225L", "MAT116", "MAT120", "MAT125", "ENG103", "ENG105", "ENG111",
};

int course_credits[] = {
    3, 1, 3, 3, 1, 3, 1, 3, 3, 3, 3, 3, 3,
};

char *course_names[] = {
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

int course_count = 13;

double calc_gp(int marks)
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

char *get_letter_grade(double gp)
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

void set_semester(int *semester)
{
    printf("Enter semester number (e.g. 261; enter 0 to stop): ");
    scanf("%d", semester);
}

void set_name(char *name)
{
    printf("Enter your name: ");
    fgets(name, COURSE_NAME_MAX, stdin);

    int len = strlen(name);

    if (len > 0 && name[len - 1] == '\n')
        name[len - 1] = '\0';
}

void print_course_list(void)
{
    for (int i = 0; i < course_count; i++)
    {
        printf("[%d]: %s - %s\n", i + 1, course_codes[i], course_names[i]);
    }

    printf("\n");
}

void add_courses_interactive(struct semester_record records[], int record_count)
{
    printf("Courses are listed below,");
    printf(" please enter the appropriate course IDs");
    printf(" indicated inside square brackets ('[]').\n");
    printf("**Enter 0 to stop adding courses and finalize the semester data.**\n\n");

    print_course_list();

    for (;;)
    {
        printf("Enter course ID to add: ");

        int course_id = -1;
        scanf("%d", &course_id);

        if (course_id == 0)
            break;

        if (course_id < 1 || course_id > course_count)
        {
            printf("Invalid course ID: %d\n", course_id);
            continue;
        }

        course_id--;

        int dup_course = 0;

        for (int i = 0; i < records[record_count].course_count; i++)
        {
            if (records[record_count].course_ids[i] == course_id)
            {
                dup_course = 1;
                printf("Course already added: [%d] (%s - %s)\n", course_id + 1, course_codes[course_id], course_names[course_id]);
                break;
            }
        }

        if (dup_course)
        {
            continue;
        }

        int marks = 0;
        printf("Enter marks for [%d] %s - %s: ", course_id + 1, course_codes[course_id], course_names[course_id]);

        scanf("%d", &marks);

        if (marks < 0 || marks > 100)
        {
            printf("Invalid marks: must be in between 0 to 100\n");
            continue;
        }

        for (int i = 0; i < record_count; i++)
        {
            for (int j = 0; j < records[i].course_count; j++)
            {
                if (records[i].course_ids[j] == course_id)
                {
                    records[i].course_ignored_flags[course_id] = 1;
                }
            }
        }

        records[record_count].course_ids[records[record_count].course_count] = course_id;
        records[record_count].course_count++;
        records[record_count].course_marks[course_id] = marks;
        records[record_count].course_ignored_flags[course_id] = 0;

        printf("Course added: %s - %s [Marks: %d/100]\n\n", course_codes[course_id], course_names[course_id], marks);

        if (records[record_count].course_count >= MAX_COURSES)
        {
            printf("Max course limit exceeded (%d), no more course entries allowed\n", records[record_count].course_count);
            break;
        }
    }
}

void print_divider(void)
{
    printf("================================================================================\n");
}

void print_transcript(char *name, struct semester_record_list *record_list)
{
    print_divider();
    printf("Name:           %s\n", name);
    print_divider();

    double wgp = 0;
    double credits = 0;

    for (int i = 0; i < record_list->count; i++)
    {
        double term_wgp = 0;
        double term_credits = 0;
        struct semester_record record = record_list->list[i];

        if (record.course_count < 1)
        {
            continue;
        }

        printf("Semester:       %d\n", record.semester);
        printf("\n");
        printf("ID\tCourse");

        int max = 0;

        for (int j = 0; j < record.course_count; j++)
        {
            int course_id = record.course_ids[j];
            int len = (int) strlen(course_codes[course_id]) + (int) strlen(course_names[course_id]);

            if (len > max)
                max = len;
        }

        max += 3;

        for (int i = 0; i < max - 4; i++)
            printf(" ");

        printf("Credits  Marks  GP  "
               "  Grade\n");

        for (int j = 0; j < record.course_count; j++)
        {
            int course_id = record.course_ids[j];
            int max_len = max - 3 - (int) strlen(course_codes[course_id]) - (int) strlen(course_names[course_id]);

            printf("[%d]\t%s - %s", course_id + 1, course_codes[course_id], course_names[course_id]);

            for (int k = 0; k < max_len; k++)
                printf(" ");

            double gp = calc_gp(record.course_marks[course_id]);
            char credit_ignored_msg[32] = "        ";

            if (record.course_ignored_flags[course_id])
            {
                strcpy(credit_ignored_msg, " [NC]   ");
            }

            printf("  %d%s%-3d    %1.2lf  %s\n", course_credits[course_id], credit_ignored_msg, record.course_marks[course_id], gp, get_letter_grade(gp));

            if (!record.course_ignored_flags[course_id])
            {
                credits += course_credits[course_id];
                wgp += gp * course_credits[course_id];
                term_credits += course_credits[course_id];
                term_wgp += gp * course_credits[course_id];
            }
        }

        double tgpa = 0.0;

        if (term_credits > 0)
        {
            tgpa = term_wgp / term_credits;
        }

        printf("\nTerm Credits:   %.0lf\n", term_credits);
        printf("TGPA:           %1.2lf (%s)\n", tgpa, get_letter_grade(tgpa));
        print_divider();
    }

    double cgpa = 0.0;

    if (credits > 0)
    {
        cgpa = wgp / credits;
    }

    printf("Credits:        %.0lf\n", credits);
    printf("CGPA:           %1.2lf (%s)\n", cgpa, get_letter_grade(cgpa));
    print_divider();
    printf("**** End of transcript ****\n");
}

int main(void)
{
    char name[COURSE_NAME_MAX];
    struct semester_record_list record_list;

    record_list.count = 0;

    memset(name, 0, sizeof(name));
    printf("Starting interactive transcript generation.\n");
    set_name(name);

    do
    {
        memset(record_list.list[record_list.count].course_ids, 0, sizeof(int) * MAX_COURSES);
        memset(record_list.list[record_list.count].course_marks, 0, sizeof(int) * MAX_COURSES);
        memset(record_list.list[record_list.count].course_ignored_flags, 0, sizeof(int) * MAX_COURSES);

        record_list.list[record_list.count].course_count = 0;

        set_semester(&record_list.list[record_list.count].semester);

        if (record_list.list[record_list.count].semester == 0)
            break;

        add_courses_interactive(record_list.list, record_list.count);

        record_list.count++;

        if (record_list.count >= SEMESTER_COUNT_MAX)
        {
            printf("Max semester limit reached, terminating.\n");
            break;
        }
    } while (1);

    printf("\n\n");
    print_transcript(name, &record_list);
    return 0;
}
