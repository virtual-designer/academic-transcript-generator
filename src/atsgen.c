#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <hpdf.h>
#include <limits.h>

#define PROMPT ">>> "

static const int COURSE_NAME_MAX = 128;

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
    fgets(name, COURSE_NAME_MAX, stdin);
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

static void pdf_error_handler(HPDF_STATUS code, HPDF_STATUS detail, void *ignored)
{
    fprintf(stderr, "libharu error: code=%04lx, detail=%ld\n", (uint64_t) code,
            (uint64_t) detail);
    exit(1);
}

static char *get_semester_string(int semester)
{
    char semester_string[128] = {0};

    if (semester < 100 || semester > 999) {
        return NULL;
    }

    int digit = semester % 10;
    char *name;

    switch (digit) {
        case 1:
            name = "Spring";
            break;

        case 2:
            name = "Summer";
            break;

        case 3:
            name = "Fall";
            break;

        default:
            return NULL;
    }

    int year = 2000 + (semester / 10);
    snprintf(semester_string, sizeof semester_string, "%s %d", name, year);
    return strdup(semester_string);
}

static void export_to_pdf(const char *name, int semester, int *added_courses,
                         int *added_course_marks, int added_course_count)
{
    char c;

    printf(PROMPT "Save as PDF? [y/N, filename=transcript.pdf]: ");
    scanf(" %c", &c);

    if (tolower(c) != 'y') {
        printf("Not saving this transcript.\n");
        return;
    }

#ifdef _WIN32
    const char path_sep = '\\';
#else /* not _WIN32 */
    const char path_sep = '/';
#endif /* _WIN32 */

    char cwd[PATH_MAX] = {0};
    getcwd(cwd, PATH_MAX);
    printf("Saving this transcript to: %s%ctranscript.pdf\n", cwd, path_sep);

    HPDF_Doc pdf = HPDF_New(&pdf_error_handler, NULL);

    if (!pdf) {
        fprintf(stderr, "Error: cannot initialize HPDF\n");
        return;
    }

    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Font font_bold = HPDF_GetFont(pdf, "Helvetica-Bold", NULL);

    const char *title = "North South University";
    const char *subtitle = "Academic Transcript";

    const float page_width = HPDF_Page_GetWidth(page);
    const float page_height = HPDF_Page_GetHeight(page);

    HPDF_Page_SetFontAndSize(page, font, 30);
    const float title_width = HPDF_Page_TextWidth(page, title);
    HPDF_Page_SetRGBFill(page, 0.0f, 123.0f / 255.0f, 1.0f);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, (page_width - title_width) / 2.0f,
                      page_height - 64, title);
    HPDF_Page_EndText(page);

    HPDF_Page_SetFontAndSize(page, font, 20);
    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    const float subtitle_width = HPDF_Page_TextWidth(page, subtitle);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, (page_width - subtitle_width) / 2.0f,
                      page_height - 95, subtitle);
    HPDF_Page_EndText(page);

    HPDF_Page_SetFontAndSize(page, font_bold, 12);

    const char *name_field = "Student name:";
    const char *semester_field = "Semester:";
    const char *sgpa_field = "SGPA:";
    const char *sgrade_field = "Semester grade:";
    const int field_off_x = 50;
    const int field_value_off_x = 140;

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, field_off_x, page_height - 150, name_field);
    HPDF_Page_EndText(page);

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, field_off_x, page_height - 170, semester_field);
    HPDF_Page_EndText(page);

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, field_off_x, page_height - 190, sgpa_field);
    HPDF_Page_EndText(page);

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, field_off_x, page_height - 210, sgrade_field);
    HPDF_Page_EndText(page);

    HPDF_Page_SetFontAndSize(page, font, 12);
    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, field_off_x + field_value_off_x, page_height - 150, name);
    HPDF_Page_EndText(page);

    char *semester_string = get_semester_string(semester);

    if (!semester_string)
        goto pdf_err;

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, field_off_x + field_value_off_x, page_height - 170, semester_string);
    HPDF_Page_EndText(page);

    const int rows = added_course_count + 1;
    const float row_height = 20;
    const float table_w = page_width - 100, table_h = row_height * rows + rows - 1;
    const float table_x = 50, table_y = page_height - 240 - table_h;

    HPDF_Page_SetLineWidth(page, 1);
    HPDF_Page_Rectangle(page, table_x, table_y, table_w, table_h);
    HPDF_Page_Stroke(page);

    HPDF_Page_SetRGBStroke(page, 0.0f, 0.0f, 0.0f);

    const int id_col_text_off_x = 5;
    const int id_col_off_x = id_col_text_off_x + 25;
    const int course_title_col_text_off_x = id_col_off_x + 5 + 1;
    const int course_title_col_off_x = course_title_col_text_off_x + 220;
    const int credits_col_text_off_x = course_title_col_off_x + 5 + 1;
    const int credits_col_off_x = credits_col_text_off_x + 60;
    const int marks_col_text_off_x = credits_col_off_x + 5 + 1;
    const int marks_col_off_x = marks_col_text_off_x + 60;
    const int gp_col_text_off_x = marks_col_off_x + 5 + 1;
    const int gp_col_off_x = gp_col_text_off_x + 40;
    const int grade_col_text_off_x = gp_col_off_x + 5 + 1;
    const int grade_col_off_x = grade_col_text_off_x + 60;
    const int cols = 6;
    const int col_off_x_list[6] = {
        id_col_off_x,
        course_title_col_off_x,
        credits_col_off_x,
        marks_col_off_x,
        gp_col_off_x,
        grade_col_off_x,
    };

    HPDF_Page_SetFontAndSize(page, font_bold, 12);

    for (int i = 1; i < rows; i++) {
        HPDF_Page_MoveTo(page, table_x, table_y + table_h - ((row_height + 1) * i));
        HPDF_Page_LineTo(page, table_w + table_x, table_y + table_h - ((row_height + 1) * i));
        HPDF_Page_Stroke(page);

        if (i == 1) {
            const int y = table_y + table_h - 15;
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + id_col_text_off_x, y, "ID");
            HPDF_Page_EndText(page);

            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + course_title_col_text_off_x, y, "Course Title");
            HPDF_Page_EndText(page);

            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + credits_col_text_off_x, y, "Credits");
            HPDF_Page_EndText(page);

            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + marks_col_text_off_x, y, "Marks");
            HPDF_Page_EndText(page);

            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + gp_col_text_off_x, y, "GP");
            HPDF_Page_EndText(page);

            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, table_x + grade_col_text_off_x, y, "Grade");
            HPDF_Page_EndText(page);
        }
    }

    for (int i = 0; i < cols - 1; i++) {
        HPDF_Page_MoveTo(page, table_x + col_off_x_list[i], table_y);
        HPDF_Page_LineTo(page, table_x + col_off_x_list[i], table_y + table_h);
        HPDF_Page_Stroke(page);
    }

    double wgp = 0;
    double credits = 0;

    HPDF_Page_SetFontAndSize(page, font, 12);

    for (int i = 0; i < added_course_count; i++) {
        int course_id = added_courses[i];
        const int y = table_y + table_h - 15 - (20 * (i + 1)) - i;

        char id_str[16] = {0};
        snprintf(id_str, sizeof id_str, "%d", course_id + 1);

        char credits_str[16] = {0};
        snprintf(credits_str, sizeof credits_str, "%d", course_credits[course_id]);

        char marks_str[16] = {0};
        snprintf(marks_str, sizeof marks_str, "%d", added_course_marks[course_id]);

        double gp = calc_gp(added_course_marks[course_id]);

        char gp_str[16] = {0};
        snprintf(gp_str, sizeof gp_str, "%1.2lf", gp);

        const char *letter_grade = get_letter_grade(gp);

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, table_x + id_col_text_off_x, y, id_str);
        HPDF_Page_EndText(page);

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, table_x + course_title_col_text_off_x, y, course_names[course_id]);
        HPDF_Page_EndText(page);

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, table_x + credits_col_text_off_x, y, credits_str);
        HPDF_Page_EndText(page);

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, table_x + marks_col_text_off_x, y, marks_str);
        HPDF_Page_EndText(page);

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, table_x + gp_col_text_off_x, y, gp_str);
        HPDF_Page_EndText(page);

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, table_x + grade_col_text_off_x, y, letter_grade);
        HPDF_Page_EndText(page);

        credits += course_credits[course_id];
        wgp += gp * course_credits[course_id];
    }

    double sgpa = wgp / credits;
    char sgpa_str[16] = {0};
    snprintf(sgpa_str, sizeof sgpa_str, "%1.2lf", sgpa);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, field_off_x + field_value_off_x, page_height - 190, sgpa_str);
    HPDF_Page_EndText(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, field_off_x + field_value_off_x, page_height - 210, get_letter_grade(sgpa));
    HPDF_Page_EndText(page);

    free(semester_string);
    HPDF_SaveToFile(pdf, "transcript.pdf");
    HPDF_Free(pdf);

    return;

pdf_err:
    free(semester_string);
    HPDF_Free(pdf);
}

int main(void)
{
    int added_courses[MAX_COURSES];
    int added_course_marks[MAX_COURSES];
    int added_course_count = 0;
    int semester = 0;
    char name[COURSE_NAME_MAX];

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
    export_to_pdf(name, semester, added_courses, added_course_marks,
                  added_course_count);
    return 0;
}
