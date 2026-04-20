/*
    atsgen.c -- Generate PDF transcripts.

    Copyright (C) 2026  Akib Elahi, Ar Rakin, Momsad Hossain.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <ctype.h>
#include <hpdf.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROMPT ">>> "
#define MAX_COURSES 100
#define COURSE_NAME_MAX 128

struct date
{
    int d, m, y;
};

struct semester_record
{
    int courses[MAX_COURSES];
    int course_marks[MAX_COURSES];
    int course_count;
    int semester;
    char *semester_string;
    double wgp;
    int credits;
    int credits_passed;
    double tgpa;
    double cgpa;
};

struct semester_records
{
    char name[COURSE_NAME_MAX];
    struct date dob;
    uint64_t id;
    int credits;
    int credits_passed;
    double wgp;
    struct semester_record **list;
    int count;
};

struct str_parts
{
    char **parts;
    size_t count;
};

static const char *course_codes[] = {
    "CSE115", "CSE115L", "CSE173", "CSE215", "CSE215L", "CSE225", "CSE225L",
    "MAT116", "MAT120",  "MAT125", "ENG103", "ENG105",  "ENG111",
};

static int course_credits[] = {
    3, 1, 3, 3, 1, 3, 1, 3, 3, 3, 3, 3, 3,
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

static void str_parts_free(struct str_parts *s_parts)
{
    for (size_t i = 0; i < s_parts->count; i++)
        free(s_parts->parts[i]);

    free(s_parts->parts);
}

static struct str_parts *str_chunk_word(const char *str, size_t limit)
{
    size_t len = strlen(str);
    char **parts = NULL;
    size_t count = 0, last_off = 0;

    for (size_t i = 0; i < len; i++)
    {
        if (!isspace(str[i]) && i + 1 < len)
        {
            continue;
        }

        if (i - last_off + 1 >= limit || i + 1 >= len)
        {
            char *part = strndup(str + last_off, i - last_off + 1);
            last_off = i + 1;

            if (!part)
                goto str_chunk_err;

            char **new_parts = realloc(parts, sizeof(char *) * (count + 1));

            if (!new_parts)
                goto str_chunk_err;

            parts = new_parts;
            parts[count++] = part;
        }
    }

    struct str_parts *str_parts = malloc(sizeof(*str_parts));

    if (!str_parts)
        goto str_chunk_err;

    str_parts->count = count;
    str_parts->parts = parts;

    return str_parts;

str_chunk_err:
    for (size_t i = 0; i < count; i++)
        free(parts[i]);

    free(parts);
    return NULL;
}

static const char *date_get_month(int month)
{
    switch (month)
    {
        case 1:
            return "January";

        case 2:
            return "February";

        case 3:
            return "March";

        case 4:
            return "April";

        case 5:
            return "May";

        case 6:
            return "June";

        case 7:
            return "July";

        case 8:
            return "August";

        case 9:
            return "September";

        case 10:
            return "October";

        case 11:
            return "November";

        case 12:
            return "December";

        default:
            return NULL;
    }
}

static bool scanf_date(struct date *date)
{
    scanf("%d/%d/%d", &date->d, &date->m, &date->y);

    if (date->m < 1 || date->m > 12)
        return false;

    int max_days = date->m >= 8   ? date->m % 2 ? 30 : 31
                   : date->m % 2  ? 31
                   : date->m == 2 ? 28
                                  : 30;

    if (date->d < 1 || date->d > max_days)
        return false;

    return true;
}

static const char *get_dept_from_id(uint64_t id)
{
    uint64_t last_2_digits = id % 100;

    switch (last_2_digits)
    {
        case 40:
        case 42:
        case 43:
        case 45:
            return "Electrical & Computer Engineering";

        case 10:
            return "Architecture";

        case 11:
            return "Law";

        case 15:
            return "English & Modern Languages";

        case 20:
            return "Economics";

        case 25:
            return "Civil & Environmental Engineering";

        case 26:
        case 27:
            return "Environmental Science & Management";

        case 30:
            return "Management";

        case 49:
        case 46:
            return "Pharmaceutical Sciences";

        case 47:
        case 48:
            return "Biochemistry & Microbiology";

        default:
            return "<unknown>";
    }
}

static const char *get_degree_from_id(uint64_t id)
{
    uint64_t last_2_digits = id % 100;

    switch (last_2_digits)
    {
        case 40:
        case 42:
            return "Bachelor of Science in Computer Science & Engineering";

        case 43:
            return "Bachelor of Science in Electrical & Electronic Engineering";

        case 10:
            return "Bachelor of Architecture";

        case 11:
            return "Bachelor of Laws";

        case 15:
            return "Bachelor of Arts in English";

        case 20:
            return "Bachelor of Science in Economics";

        case 25:
            return "Bachelor of Science in Civil & Environmental Engineering";

        case 26:
            return "Bachelor of Science in Environmental Science";

        case 27:
            return "Bachelor of Science in Environmental Management";

        case 30:
            return "Bachelor of Business Administration";

        case 45:
            return "Bachelor of Science in Electrical & Telecommunication Engineering";

        case 46:
            return "Bachelor of Pharmacy";

        case 47:
            return "Bachelor of Science in Biochemistry & Biotechnology";

        case 48:
            return "Bachelor of Science in Microbiology";

        case 49:
            return "Bachelor of Pharmacy (Professional)";

        default:
            return "<unknown>";
    }
}

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

static char *get_semester_string(int semester)
{
    char semester_string[128] = {0};

    if (semester < 100 || semester > 999)
    {
        return NULL;
    }

    int digit = semester % 10;
    char *name;

    switch (digit)
    {
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

static void set_semester(int *semester)
{
    printf(PROMPT "Enter semester number (e.g. 261; 0 to end): ");
    scanf("%d", semester);

    if (*semester == 0)
        return;

    char *semester_string = get_semester_string(*semester);

    if (!semester_string)
    {
        fprintf(stderr, "Invalid semester\n");
        exit(1);
    }

    printf("Semester set to %s\n", semester_string);
    free(semester_string);
}

static void set_name(char *name)
{
    printf(PROMPT "Enter your name: ");
    fgets(name, COURSE_NAME_MAX, stdin);
    size_t len = strlen(name);

    if (name[len - 1] == '\n')
        name[len - 1] = '\0';
}

static void set_id(uint64_t *id)
{
    printf(PROMPT "Enter your ID: ");
    scanf("%" PRIu64, id);

    if (*id < 1000000000ULL || *id > 9999999999ULL)
    {
        fprintf(stderr, "Invalid ID entered\n");
    }
}

static void set_dob(struct date *dob)
{
    printf(PROMPT "Enter your date of birth (format dd/MM/YYYY): ");

    if (!scanf_date(dob))
    {
        fprintf(stderr, "Invalid date entered\n");
    }
}

static void print_course_list(void)
{
    for (int i = 0; i < course_count; i++)
    {
        printf("[%d]: %s - %s\n", i + 1, course_codes[i], course_names[i]);
    }

    printf("\n");
}

static void add_courses_interactive(int *added_courses, int *added_course_marks,
                                    int *added_course_count)
{
    printf("Courses are listed below,");
    printf(" please enter the appropriate course IDs");
    printf(" indicated inside square brackets '[]').\n");
    printf("**Enter 0 to stop adding courses and finalize the semester "
           "record.**\n\n");

    print_course_list();

    for (;;)
    {
        printf(PROMPT "Enter course ID to add: ");

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

        int marks = 0;
        printf(PROMPT "Enter marks for [%d] %s - %s: ", course_id + 1,
               course_codes[course_id], course_names[course_id]);

        scanf("%d", &marks);

        if (marks < 0 || marks > 100)
        {
            printf("Invalid marks: must be in between 0 to 100\n");
            continue;
        }

        added_courses[*added_course_count] = course_id;
        (*added_course_count)++;
        added_course_marks[course_id] = marks;

        printf("Course added: %s - %s [Marks: %d/100]\n\n",
               course_codes[course_id], course_names[course_id], marks);

        if ((*added_course_count) >= MAX_COURSES)
        {
            printf("Max course limit exceeded (%d), no more course entries "
                   "allowed\n",
                   *added_course_count);
            break;
        }
    }
}

static void pdf_error_handler(HPDF_STATUS code, HPDF_STATUS detail,
                              void *ignored)
{
    (void) ignored;
    fprintf(stderr, "libharu error: code=%" PRIu64 ", detail=%" PRIu64 "\n",
            (uint64_t) code, (uint64_t) detail);
    exit(1);
}

#define PDF_FONT_NORMAL HPDF_GetFont(pdf, "Times-Roman", NULL)
#define PDF_FONT_BOLD HPDF_GetFont(pdf, "Times-Bold", NULL)
#define PDF_FONT_BOLD_ITALIC HPDF_GetFont(pdf, "Times-BoldItalic", NULL)

static void pdf_draw_header_title(HPDF_Doc pdf, HPDF_Page page)
{
    const HPDF_REAL page_h = HPDF_Page_GetHeight(page);
    const HPDF_REAL page_w = HPDF_Page_GetWidth(page);
    const HPDF_REAL size_hi = 30;
    const HPDF_REAL size_lo = 22;

    const char *text_N = "N";
    const char *text_S = "S";
    const char *text_U = "U";
    const char *text_orth = " O R T H  ";
    const char *text_outh = " O U T H  ";
    const char *text_niversity = " N I V E R S I T Y";

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, size_hi);

    const HPDF_REAL text_N_w = HPDF_Page_TextWidth(page, text_N);
    const HPDF_REAL text_S_w = HPDF_Page_TextWidth(page, text_S);
    const HPDF_REAL text_U_w = HPDF_Page_TextWidth(page, text_U);

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, size_lo);

    const HPDF_REAL text_orth_w = HPDF_Page_TextWidth(page, text_orth);
    const HPDF_REAL text_outh_w = HPDF_Page_TextWidth(page, text_outh);
    const HPDF_REAL text_niversity_w =
        HPDF_Page_TextWidth(page, text_niversity);
    const HPDF_REAL text_w_total = text_N_w + text_S_w + text_U_w +
                                   text_orth_w + text_outh_w + text_niversity_w;

    const HPDF_REAL title_x = (page_w - text_w_total) / 2.0;
    const HPDF_REAL title_y = page_h - 50;

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, size_hi);
    HPDF_Page_SetRGBFill(page, 0.0f, 123.0f / 255.0f, 1.0f);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, title_x, title_y, text_N);
    HPDF_Page_EndText(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, title_x + text_N_w + text_orth_w, title_y, text_S);
    HPDF_Page_EndText(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page,
                      title_x + text_N_w + text_orth_w + text_S_w + text_outh_w,
                      title_y, text_U);
    HPDF_Page_EndText(page);

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, size_lo);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, title_x + text_N_w, title_y, text_orth);
    HPDF_Page_EndText(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, title_x + text_N_w + text_orth_w + text_S_w,
                      title_y, text_outh);
    HPDF_Page_EndText(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page,
                      title_x + text_N_w + text_orth_w + text_S_w +
                          text_outh_w + text_U_w,
                      title_y, text_niversity);
    HPDF_Page_EndText(page);

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
}

static void pdf_draw_header_slogan(HPDF_Doc pdf, HPDF_Page page)
{
    const HPDF_REAL page_h = HPDF_Page_GetHeight(page);
    const HPDF_REAL page_w = HPDF_Page_GetWidth(page);
    const HPDF_REAL size_hi = 11.25;
    const HPDF_REAL size_lo = 9.25;

    const char *text_T = "T";
    const char *text_F = "F";
    const char *text_P = "P";
    const char *text_U = "U";
    const char *text_I = "I";
    const char *text_B = "B";

    const char *text_he = "  H  E    ";
    const char *text_irst = "  I  R  S  T    ";
    const char *text_rivate = "  R  I  V  A  T  E    ";
    const char *text_niversity = "  N  I  V  E  R  S  I  T  Y    ";
    const char *text_n = "  N    ";
    const char *text_angladesh = "  A  N  G  L  A  D  E  S  H";

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, size_hi);

    const HPDF_REAL text_T_w = HPDF_Page_TextWidth(page, text_T);
    const HPDF_REAL text_F_w = HPDF_Page_TextWidth(page, text_F);
    const HPDF_REAL text_P_w = HPDF_Page_TextWidth(page, text_P);
    const HPDF_REAL text_U_w = HPDF_Page_TextWidth(page, text_U);
    const HPDF_REAL text_I_w = HPDF_Page_TextWidth(page, text_I);
    const HPDF_REAL text_B_w = HPDF_Page_TextWidth(page, text_B);

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, size_lo);

    const HPDF_REAL text_he_w = HPDF_Page_TextWidth(page, text_he);
    const HPDF_REAL text_irst_w = HPDF_Page_TextWidth(page, text_irst);
    const HPDF_REAL text_rivate_w = HPDF_Page_TextWidth(page, text_rivate);
    const HPDF_REAL text_niversity_w =
        HPDF_Page_TextWidth(page, text_niversity);
    const HPDF_REAL text_n_w = HPDF_Page_TextWidth(page, text_n);
    const HPDF_REAL text_angladesh_w =
        HPDF_Page_TextWidth(page, text_angladesh);
    const HPDF_REAL text_w_total =
        text_T_w + text_F_w + text_P_w + text_U_w + text_I_w + text_B_w +
        text_he_w + text_irst_w + text_rivate_w + text_niversity_w + text_n_w +
        text_angladesh_w;

    const HPDF_REAL slogan_x = (page_w - text_w_total) / 2.0 - 2.0;
    const HPDF_REAL slogan_y = page_h - 64;

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, size_hi);
    HPDF_Page_SetRGBFill(page, 0.4f, 0.4f, 0.4f);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, slogan_x, slogan_y, text_T);
    HPDF_Page_TextOut(page, slogan_x + text_T_w + text_he_w, slogan_y, text_F);
    HPDF_Page_TextOut(page,
                      slogan_x + text_T_w + text_he_w + text_F_w + text_irst_w,
                      slogan_y, text_P);
    HPDF_Page_TextOut(page,
                      slogan_x + text_T_w + text_he_w + text_F_w + text_irst_w +
                          text_P_w + text_rivate_w,
                      slogan_y, text_U);
    HPDF_Page_TextOut(page,
                      slogan_x + text_T_w + text_he_w + text_F_w + text_irst_w +
                          text_P_w + text_rivate_w + text_U_w +
                          text_niversity_w,
                      slogan_y, text_I);
    HPDF_Page_TextOut(page,
                      slogan_x + text_T_w + text_he_w + text_F_w + text_irst_w +
                          text_P_w + text_rivate_w + text_U_w +
                          text_niversity_w + text_I_w + text_n_w,
                      slogan_y, text_B);
    HPDF_Page_EndText(page);

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, size_lo);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, slogan_x + text_T_w, slogan_y, text_he);
    HPDF_Page_TextOut(page, slogan_x + text_T_w + text_he_w + text_F_w,
                      slogan_y, text_irst);
    HPDF_Page_TextOut(page,
                      slogan_x + text_T_w + text_he_w + text_F_w + text_irst_w +
                          text_P_w,
                      slogan_y, text_rivate);
    HPDF_Page_TextOut(page,
                      slogan_x + text_T_w + text_he_w + text_F_w + text_irst_w +
                          text_P_w + text_rivate_w + text_U_w,
                      slogan_y, text_niversity);
    HPDF_Page_TextOut(page,
                      slogan_x + text_T_w + text_he_w + text_F_w + text_irst_w +
                          text_P_w + text_rivate_w + text_U_w +
                          text_niversity_w + text_I_w,
                      slogan_y, text_n);
    HPDF_Page_TextOut(page,
                      slogan_x + text_T_w + text_he_w + text_F_w + text_irst_w +
                          text_P_w + text_rivate_w + text_U_w +
                          text_niversity_w + text_I_w + text_n_w + text_B_w,
                      slogan_y, text_angladesh);
    HPDF_Page_EndText(page);

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
}

static void pdf_draw_header_addr(HPDF_Doc pdf, HPDF_Page page)
{
    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, 11.5);

    const HPDF_REAL page_h = HPDF_Page_GetHeight(page);
    const HPDF_REAL page_w = HPDF_Page_GetWidth(page);
    const char *addr_text1 = "Plot # 15, Block # B, Bashundhara, Dhaka-1229, "
                             "Bangladesh, Phone 880 (2) 8852000,";
    const char *addr_text2 =
        "Fax: 880 (2) 8852016, Email: controller@northsouth.edu, Website: "
        "www.northsouth.edu";
    const HPDF_REAL addr_text1_w = HPDF_Page_TextWidth(page, addr_text1);
    const HPDF_REAL addr_text2_w = HPDF_Page_TextWidth(page, addr_text2);
    const HPDF_REAL addr_text1_x = (page_w - addr_text1_w) / 2.0;
    const HPDF_REAL addr_text2_x = (page_w - addr_text2_w) / 2.0;
    const HPDF_REAL addr_text1_y = page_h - 78;
    const HPDF_REAL addr_text2_y = page_h - 89.5;

    HPDF_Page_SetRGBFill(page, 0.4f, 0.4f, 0.4f);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, addr_text1_x, addr_text1_y, addr_text1);
    HPDF_Page_TextOut(page, addr_text2_x, addr_text2_y, addr_text2);
    HPDF_Page_EndText(page);

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
}

static void pdf_draw_header_office(HPDF_Doc pdf, HPDF_Page page)
{
    HPDF_Page_SetFontAndSize(page, PDF_FONT_BOLD_ITALIC, 16);

    const HPDF_REAL page_h = HPDF_Page_GetHeight(page);
    const HPDF_REAL page_w = HPDF_Page_GetWidth(page);
    const char *office_text = "Office of The Controller of Examinations";
    const HPDF_REAL office_text_w = HPDF_Page_TextWidth(page, office_text);
    const HPDF_REAL office_text_x = (page_w - office_text_w) / 2.0;
    const HPDF_REAL office_text_y = page_h - 110;

    HPDF_Page_SetRGBFill(page, 0.4f, 0.4f, 0.4f);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, office_text_x, office_text_y, office_text);
    HPDF_Page_EndText(page);

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
}

static void pdf_draw_header(HPDF_Doc pdf, HPDF_Page page)
{
    pdf_draw_header_title(pdf, page);
    pdf_draw_header_slogan(pdf, page);
    pdf_draw_header_addr(pdf, page);
    pdf_draw_header_office(pdf, page);
}

static bool pdf_draw_header_top_data(HPDF_Doc pdf, HPDF_Page page,
                                     struct semester_records *records)
{
    const HPDF_REAL page_h = HPDF_Page_GetHeight(page);
    const HPDF_REAL page_w = HPDF_Page_GetWidth(page);
    const HPDF_REAL x = page_w / 2 + 25;
    const HPDF_REAL y = page_h - 150;
    const HPDF_REAL val_off = 95;
    int iota = 0;

    HPDF_Page_SetFontAndSize(page, PDF_FONT_BOLD, 10);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, x, y + 16, "Demo Transcript");
    HPDF_Page_TextOut(page, x + val_off, y + 16, "16 Apr 2026");
    HPDF_Page_EndText(page);

    HPDF_Page_SetFontAndSize(page, PDF_FONT_BOLD, 7);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, x, y, "Student Name:");
    HPDF_Page_TextOut(page, x, y - (iota += 10), "Student ID:");
    HPDF_Page_TextOut(page, x, y - (iota += 10), "Date of Birth:");
    HPDF_Page_TextOut(page, x, y - (iota += 10), "Degree Conferred:");
    HPDF_Page_EndText(page);

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, 7);

    iota = 0;
    char id_str[64] = {0};
    snprintf(id_str, sizeof id_str, "%07" PRIu64 " %01" PRIu64 " %02" PRIu64 "",
             records->id / 1000, (records->id / 100) % 10, records->id % 100);
    char dob_str[64] = {0};
    snprintf(dob_str, sizeof dob_str, "%02d %.3s %04d", records->dob.d,
             date_get_month(records->dob.m), records->dob.y);

    struct str_parts *degree_parts =
        str_chunk_word(get_degree_from_id(records->id), 30);

    if (!degree_parts)
    {
        return false;
    }

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, x + val_off, y, records->name);
    HPDF_Page_TextOut(page, x + val_off, y - (iota += 10), id_str);
    HPDF_Page_TextOut(page, x + val_off, y - (iota += 10), dob_str);

    for (size_t i = 0; i < degree_parts->count; i++)
    {
        HPDF_Page_TextOut(page, x + val_off, y - (iota += 10),
                          degree_parts->parts[i]);
    }

    HPDF_Page_EndText(page);
    str_parts_free(degree_parts);

    return true;
}

static void pdf_add_record_hr(HPDF_Page page, HPDF_REAL *x_off,
                              HPDF_REAL *y_off)
{
    const HPDF_REAL page_w = HPDF_Page_GetWidth(page);

    HPDF_Page_SetRGBStroke(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_SetLineWidth(page, 0.5f);
    HPDF_Page_MoveTo(page, *x_off, *y_off);
    HPDF_Page_LineTo(page, (*x_off) + ((page_w - 150) / 2), *y_off);
    HPDF_Page_Stroke(page);

    *y_off -= 1.0f;
}

static int pdf_add_record(HPDF_Doc pdf, HPDF_Page page,
                          struct semester_record *record, HPDF_REAL *x_off,
                          HPDF_REAL *y_off)
{
    HPDF_REAL y_check = 9.0f + 1.0f + (record->course_count * (9.0f + 1.0f)) +
                        9.0f + 1.0f + 14.5f;

    for (int i = 0; i < record->course_count; i++)
    {
        const int id = record->courses[i];
        const char *course_title = course_names[id];
        struct str_parts *title_parts = str_chunk_word(course_title, 25);

        if (!title_parts)
            return 1;

        int iota = 8 * (int) title_parts->count;
        str_parts_free(title_parts);
        y_check += iota;
    }

    if ((*y_off) < y_check + 50)
        return -1;

    HPDF_Page_SetFontAndSize(page, PDF_FONT_BOLD, 7);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, *x_off, *y_off, record->semester_string);
    HPDF_Page_EndText(page);

    *y_off -= 3.0f;
    pdf_add_record_hr(page, x_off, y_off);
    *y_off -= 6.0f;

    const HPDF_REAL course_code_off = 0.0f;
    const HPDF_REAL course_title_off = 30.0f;
    const HPDF_REAL credits_off = 145.0f;
    const HPDF_REAL grade_off = 160.0f;
    const HPDF_REAL gp_off = 175.0f;
    const HPDF_REAL credits_completed_off = 195.0f;
    const HPDF_REAL credits_passed_off = 210.0f;

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, *x_off + course_code_off, *y_off, "Course");
    HPDF_Page_TextOut(page, *x_off + course_title_off, *y_off, "Course Title");
    HPDF_Page_TextOut(page, *x_off + credits_off, *y_off, "Cr.");
    HPDF_Page_TextOut(page, *x_off + grade_off, *y_off, "Gr.");
    HPDF_Page_TextOut(page, *x_off + gp_off, *y_off, "GP");
    HPDF_Page_TextOut(page, *x_off + credits_completed_off, *y_off, "CC");
    HPDF_Page_TextOut(page, *x_off + credits_passed_off, *y_off, "CP");
    HPDF_Page_EndText(page);

    HPDF_Page_SetFontAndSize(page, PDF_FONT_NORMAL, 7);

    for (int i = 0; i < record->course_count; i++)
    {
        *y_off -= 3.0f;
        pdf_add_record_hr(page, x_off, y_off);
        *y_off -= 6.0f;

        const int id = record->courses[i];
        const char *course_code = course_codes[id];
        const char *course_title = course_names[id];
        int iota = -8;

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, *x_off + course_code_off, *y_off, course_code);

        struct str_parts *title_parts = str_chunk_word(course_title, 25);

        if (!title_parts)
            return 1;

        for (size_t j = 0; j < title_parts->count; j++)
        {
            HPDF_Page_TextOut(page, *x_off + course_title_off,
                              *y_off - (iota += 8), title_parts->parts[j]);
        }

        str_parts_free(title_parts);

        char credits_str[16] = {0};
        snprintf(credits_str, sizeof credits_str, "%d", course_credits[id]);

        const double gp = calc_gp(record->course_marks[id]);
        const char *grade_str = get_letter_grade(gp);

        char gp_str[16] = {0};
        snprintf(gp_str, sizeof gp_str, "%1.2lf", gp);

        HPDF_Page_TextOut(page, *x_off + credits_off, *y_off, credits_str);
        HPDF_Page_TextOut(page, *x_off + grade_off, *y_off, grade_str);
        HPDF_Page_TextOut(page, *x_off + gp_off, *y_off, gp_str);
        HPDF_Page_TextOut(page, *x_off + credits_completed_off, *y_off,
                          credits_str);
        HPDF_Page_TextOut(page, *x_off + credits_passed_off, *y_off,
                          gp < 1.0 ? "0" : credits_str);
        HPDF_Page_EndText(page);

        *y_off -= iota;
    }

    *y_off -= 3.0f;
    pdf_add_record_hr(page, x_off, y_off);
    *y_off -= 6.0f;

    char semester_credits_str[64] = {0};
    snprintf(semester_credits_str, sizeof semester_credits_str,
             "Semester Credits: %d", record->credits);

    char tgpa_str[64] = {0};
    snprintf(tgpa_str, sizeof tgpa_str, "TGPA: %1.2lf", record->tgpa);

    char cgpa_str[64] = {0};
    snprintf(cgpa_str, sizeof cgpa_str, "CGPA: %1.2lf", record->cgpa);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, *x_off + course_code_off, *y_off,
                      semester_credits_str);
    HPDF_Page_TextOut(page, *x_off + course_code_off + 85, *y_off, tgpa_str);
    HPDF_Page_TextOut(page, *x_off + course_code_off + 189, *y_off, cgpa_str);
    HPDF_Page_EndText(page);

    *y_off -= 3.0f;
    pdf_add_record_hr(page, x_off, y_off);
    *y_off -= 11.5f;

    return 0;
}

static HPDF_Page pdf_add_page(HPDF_Doc pdf)
{
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    return page;
}

static void pdf_add_summary(HPDF_Doc pdf, HPDF_Page page,
                            struct semester_records *records, HPDF_REAL *x_off,
                            HPDF_REAL *y_off)
{
    const HPDF_REAL page_w = HPDF_Page_GetWidth(page);

    HPDF_Page_SetFontAndSize(page, PDF_FONT_BOLD, 7);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, *x_off, *y_off, "Summary");
    HPDF_Page_EndText(page);

    *y_off -= 3.0f;
    pdf_add_record_hr(page, x_off, y_off);
    *y_off -= 9.0f;

    char total_credits_str[64] = {0};
    char total_credits_passed_str[64] = {0};
    char total_gp_str[16] = {0};
    char total_cgpa_str[16] = {0};

    snprintf(total_credits_str, sizeof total_credits_str, "%d", records->credits);
    snprintf(total_credits_passed_str, sizeof total_credits_passed_str, "%d", records->credits_passed);
    snprintf(total_gp_str, sizeof total_gp_str, "%1.2lf", records->wgp);
    snprintf(total_cgpa_str, sizeof total_cgpa_str, "%1.2lf", records->wgp / records->credits);

    int iota = 8;

    HPDF_Page_BeginText(page);

    HPDF_Page_TextOut(page, *x_off, *y_off + (iota -= 8), "Total Credits Counted:");
    HPDF_Page_TextOut(page, *x_off, *y_off + (iota -= 8), "Total Credits Passed:");
    HPDF_Page_TextOut(page, *x_off, *y_off + (iota -= 8), "Total Grade Points:");
    HPDF_Page_TextOut(page, *x_off, *y_off + (iota -= 8), "Cumulative Grade Point Average:");

    iota = 8;

    const HPDF_REAL val_off = 120.0f;

    HPDF_Page_TextOut(page, *x_off + val_off, *y_off + (iota -= 8), total_credits_str);
    HPDF_Page_TextOut(page, *x_off + val_off, *y_off + (iota -= 8), total_credits_passed_str);
    HPDF_Page_TextOut(page, *x_off + val_off, *y_off + (iota -= 8), total_gp_str);
    HPDF_Page_TextOut(page, *x_off + val_off, *y_off + (iota -= 8), total_cgpa_str);

    HPDF_Page_EndText(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, *x_off + 22.5f, *y_off + (iota -= 30), " ***************** End of Transcript ***************** ");
    HPDF_Page_EndText(page);

    HPDF_Page_SetRGBStroke(page, 0.0f, 0.0f, 0.0f);

    HPDF_Page_MoveTo(page, 50.0f, *y_off + (iota -= 60));
    HPDF_Page_LineTo(page, 150.0f, *y_off + iota);
    HPDF_Page_Stroke(page);

    char dept_chair_str[256] = "Chair, Dept. of ";
    strcat(dept_chair_str, get_dept_from_id(records->id));

    HPDF_REAL text_w = HPDF_Page_TextWidth(page, dept_chair_str);

    HPDF_Page_MoveTo(page, page_w - 50.0f - text_w - 20.0f, *y_off + iota);
    HPDF_Page_LineTo(page, page_w - 50.0f, *y_off + iota);
    HPDF_Page_Stroke(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 60.0f, *y_off + (iota -= 8), "Controller of Examinations");
    HPDF_Page_TextOut(page, page_w - 50.0f - text_w - 10.0f, *y_off + iota, dept_chair_str);
    HPDF_Page_EndText(page);
}

static bool pdf_add_records(HPDF_Doc pdf, struct semester_records *records)
{
    HPDF_Page page = pdf_add_page(pdf);

    if (!page)
        return false;

    pdf_draw_header(pdf, page);

    if (!pdf_draw_header_top_data(pdf, page, records))
        return false;

    const HPDF_REAL page_h = HPDF_Page_GetHeight(page);
    const HPDF_REAL page_w = HPDF_Page_GetWidth(page);
    HPDF_REAL x_off = 50;
    HPDF_REAL y_off = page_h - 230;
    bool first_page = true;

    for (int i = 0; i < records->count; i++)
    {
        int ret = pdf_add_record(pdf, page, records->list[i], &x_off, &y_off);

        if (ret == -1)
        {
            if (x_off > 50)
            {
                page = pdf_add_page(pdf);

                if (!page)
                    return false;

                x_off = 50;
                first_page = false;
            }
            else
            {
                x_off = (page_w - 100) / 2 + 70;
            }

            y_off = page_h - (first_page ? 230 : 50);
            i--;
            continue;
        }
    }

    if (y_off < 220)
    {
        page = pdf_add_page(pdf);

        if (!page)
            return false;

        x_off = 50;
    }

    pdf_add_summary(pdf, page, records, &x_off, &y_off);
    return true;
}

static void export_to_pdf(struct semester_records *records)
{
    char c;

    printf(PROMPT "Save as PDF? [y/N, filename=transcript.pdf]: ");
    scanf(" %c", &c);

    if (tolower(c) != 'y')
    {
        printf("Not saving this transcript.\n");
        return;
    }

#ifdef _WIN32
    const char path_sep = '\\';
#else  /* not _WIN32 */
    const char path_sep = '/';
#endif /* _WIN32 */

    char cwd[PATH_MAX] = {0};
    getcwd(cwd, PATH_MAX);
    printf("Saving this transcript to: %s%ctranscript.pdf\n", cwd, path_sep);

    HPDF_Doc pdf = HPDF_New(&pdf_error_handler, NULL);

    if (!pdf)
    {
        fprintf(stderr, "Error: cannot initialize HPDF\n");
        return;
    }

    if (!pdf_add_records(pdf, records))
        goto pdf_err;

    HPDF_SaveToFile(pdf, "transcript.pdf");
    HPDF_Free(pdf);

    return;

pdf_err:
    fprintf(stderr, "An error has occurred\n");
    HPDF_Free(pdf);
}

static struct semester_records *semester_records_init(void)
{
    struct semester_records *records = calloc(1, sizeof(*records));

    if (!records)
        return NULL;

    return records;
}

static struct semester_record *semester_record_init(int semester,
                                                    const int *courses,
                                                    const int *marks,
                                                    int course_count)
{
    struct semester_record *record = calloc(1, sizeof(*record));

    if (!record)
        return NULL;

    record->semester = semester;
    record->semester_string = get_semester_string(semester);

    if (!record->semester_string)
    {
        free(record);
        return NULL;
    }

    record->course_count = course_count;

    for (int i = 0; i < course_count; i++)
    {
        int id = courses[i];
        int credits = course_credits[id];
        double gp = calc_gp(marks[id]);
        record->credits += credits;
        record->credits_passed += gp < 1.0 ? 0 : credits;
        record->wgp += gp * credits;
        record->courses[i] = id;
        record->course_marks[id] = marks[id];
    }

    record->tgpa = record->wgp / record->credits;
    return record;
}

static struct semester_record *
semester_records_add(struct semester_records *records, int semester,
                     const int *courses, const int *marks, int course_count)
{
    struct semester_record **list =
        realloc(records->list, sizeof(*list) * (records->count + 1));

    if (!list)
        return NULL;

    records->list = list;

    struct semester_record *record =
        semester_record_init(semester, courses, marks, course_count);

    if (!record)
        return NULL;

    records->list[records->count++] = record;
    records->wgp += record->wgp;
    records->credits += record->credits;
    records->credits_passed += record->credits_passed;
    record->cgpa = records->wgp / records->credits;

    return record;
}

static void semester_record_free(struct semester_record *record)
{
    free(record->semester_string);
    free(record);
}

static void semester_records_free(struct semester_records *records)
{
    for (int i = 0; i < records->count; i++)
        semester_record_free(records->list[i]);

    free(records->list);
    free(records);
}

static void semester_record_print(struct semester_record *record)
{
    char *semester_string = get_semester_string(record->semester);

    printf("Semester: %s\n", semester_string);
    printf("TGPA:     %1.2lf (%s)\n", record->tgpa,
           get_letter_grade(record->tgpa));
    printf("CGPA:     %1.2lf (%s)\n", record->cgpa,
           get_letter_grade(record->cgpa));
    printf("\n");
    printf("\nID\tCourse");

    int max = 0;

    for (int i = 0; i < record->course_count; i++)
    {
        int course_id = record->courses[i];
        int len = (int) strlen(course_codes[course_id]) +
                  (int) strlen(course_names[course_id]);

        if (len > max)
            max = len;
    }

    max += 3;

    for (int i = 0; i < max - 4; i++)
        printf(" ");

    printf("Credits  Marks  GP  "
           "  Grade\n");

    free(semester_string);

    for (int i = 0; i < record->course_count; i++)
    {
        int course_id = record->courses[i];
        printf("[%d]\t%s - %s", course_id + 1, course_codes[course_id],
               course_names[course_id]);

        for (int j = 0; j < max - 3 - (int) strlen(course_codes[course_id]) -
                                (int) strlen(course_names[course_id]);
             j++)
            printf(" ");

        double gp = calc_gp(record->course_marks[course_id]);

        printf("  %d        %-3d    %1.2lf  %s\n", course_credits[course_id],
               record->course_marks[course_id], gp, get_letter_grade(gp));
    }

    printf("\n");
}

static void semester_records_print(struct semester_records *records)
{
    double cgpa = records->wgp / records->credits;

    printf("Student Name:      %s\n", records->name);
    printf("Student ID:        %" PRIu64 "\n", records->id);
    printf("Date of Birth:     %0d %.3s %04d\n", records->dob.d,
           date_get_month(records->dob.m), records->dob.y);
    printf("Degree Conferred:  %s\n", get_degree_from_id(records->id));
    printf("CGPA:              %1.2lf (%s)\n", cgpa, get_letter_grade(cgpa));
    printf("\n");

    for (int i = 0; i < records->count; i++)
        semester_record_print(records->list[i]);
}

int main(void)
{
    printf("Starting interactive transcript generation.\n");

    struct semester_records *records = semester_records_init();

    if (!records)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    set_name(records->name);
    set_id(&records->id);
    set_dob(&records->dob);

    for (;;)
    {
        int added_courses[MAX_COURSES];
        int added_course_marks[MAX_COURSES];
        int added_course_count = 0;
        int semester = 0;

        memset(added_courses, 0, sizeof(added_courses));
        memset(added_course_marks, 0, sizeof(added_course_marks));

        set_semester(&semester);

        if (semester == 0)
            break;

        add_courses_interactive(added_courses, added_course_marks,
                                &added_course_count);
        semester_records_add(records, semester, added_courses,
                             added_course_marks, added_course_count);
        printf("\n");
    }

    semester_records_print(records);
    export_to_pdf(records);
    semester_records_free(records);
    return 0;
}
