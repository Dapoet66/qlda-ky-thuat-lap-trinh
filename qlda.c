// He thong Quan ly Cong viec Du an - Mon Ky thuat Lap trinh - HUST
/* ============================================================
  Ki Thuat Lap Trinh - MI3310 
  Chu de 10: Quan ly Cong Viec Du An
   ============================================================ */

/* ============================================================
   PHAN 1: INCLUDE & CONST & STRUCT
   ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* ---------- CONST ---------- */
#define MAX_ID       20
#define MAX_NAME    100
#define MAX_DATE     12
#define MAX_ROLE     20
#define MAX_STATUS   20

#define FILE_DUAN       "duan.txt"
#define FILE_THANHVIEN  "thanhvien.txt"
#define FILE_TIMESHEET  "timesheet.txt"

#define GIA_BA      50000.0f
#define GIA_DEV     80000.0f
#define GIA_TEST    60000.0f
#define GIA_DESIGN  70000.0f

/* ---------- Struct: Thanh Vien ---------- */
typedef struct ThanhVien {
    char maTV[MAX_ID];
    char hoTen[MAX_NAME];
    char vaiTro[MAX_ROLE];   /* BA / Dev / Test / Design */
    int  trangThai;          /* 1: Active, 0: Deleted (Soft Delete) */
    struct ThanhVien *next;
} ThanhVien;

/* ---------- Struct: Du An ---------- */
typedef struct DuAn {
    char maDuAn[MAX_ID];
    char tenDuAn[MAX_NAME];
    char ngayBatDau[MAX_DATE];
    char ngayKetThuc[MAX_DATE];
    char khachHang[MAX_NAME];
    float nganSach;
    ThanhVien *danhSachTV;
    struct DuAn *next;
} DuAn;

/* ---------- Struct: Timesheet ---------- */
typedef struct Timesheet {
    char maCongViec[MAX_ID];
    char tenCongViec[MAX_NAME];
    char loaiHinh[MAX_ROLE];     /* BA / Design / Dev / Test */
    char maTV[MAX_ID];
    char maDuAn[MAX_ID];
    char ngayLam[MAX_DATE];
    float soGio;
    char trangThai[MAX_STATUS];  /* Done / In Progress */
    struct Timesheet *next;
} Timesheet;

/* ---------- Struct: He Thong ---------- */
typedef struct {
    DuAn      *headDuAn;
    ThanhVien *headTV;
    Timesheet *headTS;
} HeThong;


/* ============================================================
   PHAN 2: UTILS & Defensive Programming
   ============================================================ */

void xoaBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void inDuongKe(int n) {
    int i;
    for (i = 0; i < n; i++) putchar('-');
    putchar('\n');
}

int docChuoi(char *buf, int maxLen) {
    if (!fgets(buf, maxLen, stdin)) { buf[0] = '\0'; return 0; }
    int len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0'; /* cat '\n' cuoi, stdin sach */
    else
        xoaBuffer();   /* buffer day — don phan thua trong stdin */
    return (buf[0] != '\0') ? 1 : 0;
}

/* Kiem tra dinh dang va logic ngày DD/MM/YYYY
   Xu ly dung so ngay trong thang va nam nhuan */
int hopLeNgay(const char *ngay) {
    int i;
    if (!ngay || strlen(ngay) != 10) return 0;
    if (ngay[2] != '/' || ngay[5] != '/') return 0;
    
    /* Kiem tra tat ca vi tri con lai la chu so */
    for (i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)ngay[i])) return 0;
    }
    int d = (ngay[0]-'0')*10 + (ngay[1]-'0');
    int m = (ngay[3]-'0')*10 + (ngay[4]-'0');
    int y = (ngay[6]-'0')*1000 + (ngay[7]-'0')*100
          + (ngay[8]-'0')*10   + (ngay[9]-'0');
    if (m < 1 || m > 12)       return 0;
    if (y < 1900 || y > 2100)  return 0;
    int ngayTrongThang[] = {0, 31, 28, 31, 30, 31, 30,
                                31, 31, 30, 31, 30, 31};
                                
    /* Dieu chinh thang 2 neu la nam nhuan */
    if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
        ngayTrongThang[2] = 29;
    if (d < 1 || d > ngayTrongThang[m]) return 0;
    return 1;
}

/* Chuyen DD/MM/YYYY sang YYYYMMDD de so sanh thu tu ngay */
long chuyenNgayThanhSo(const char *ngay) {
    int d = (ngay[0]-'0')*10 + (ngay[1]-'0');
    int m = (ngay[3]-'0')*10 + (ngay[4]-'0');
    int y = (ngay[6]-'0')*1000 + (ngay[7]-'0')*100
          + (ngay[8]-'0')*10   + (ngay[9]-'0');
    return (long)y * 10000L + m * 100L + d;
}

/* Kiem tra vai tro hop le — PHAN BIET hoa thuong */
int hopLeVaiTro(const char *vaiTro) {
    if (!vaiTro) return 0;
    return (strcmp(vaiTro, "BA")     == 0 ||
            strcmp(vaiTro, "Dev")    == 0 ||
            strcmp(vaiTro, "Test")   == 0 ||
            strcmp(vaiTro, "Design") == 0);
}

int hopLeTrangThai(const char *tt) {
    if (!tt) return 0;
    return (strcmp(tt, "Done") == 0 || strcmp(tt, "In Progress") == 0); /* chi chap nhan Done / In Progress */
}

float donGiaVaiTro(const char *vaiTro) {   
    if (!vaiTro)                       return 0.0f;
    if (strcmp(vaiTro, "BA")     == 0) return GIA_BA;
    if (strcmp(vaiTro, "Dev")    == 0) return GIA_DEV;             /* Tra ve don gia theo don vi VND */
    if (strcmp(vaiTro, "Test")   == 0) return GIA_TEST;
    if (strcmp(vaiTro, "Design") == 0) return GIA_DESIGN;
    return 0.0f;
}


/* ============================================================
   PHAN 3: QUAN LY THANH VIEN & DU AN
   ============================================================ */

ThanhVien *taoThanhVien(const char *maTV, const char *hoTen,
                        const char *vaiTro) {
    assert(maTV && hoTen && vaiTro);
    ThanhVien *tv = (ThanhVien *)malloc(sizeof(ThanhVien));
    if (!tv) { fprintf(stderr, "Loi bo nho!\n"); return NULL; }
    strncpy(tv->maTV,   maTV,   MAX_ID-1);   tv->maTV[MAX_ID-1]    = '\0';
    strncpy(tv->hoTen,  hoTen,  MAX_NAME-1); tv->hoTen[MAX_NAME-1]  = '\0';
    strncpy(tv->vaiTro, vaiTro, MAX_ROLE-1); tv->vaiTro[MAX_ROLE-1] = '\0';
    tv->trangThai = 1;   /* Mac dinh Active khi moi tao */
    tv->next = NULL;
    return tv;
}

/* tim theo maTV, tra ve node du Active hay Deleted.
   Dung noi bo cho kiem tra trung khi them moi va tinh chi phi TimeSheet lich su */
static ThanhVien *timTVKeCaGomXoa(DuAn *da, const char *maTV) {
    assert(da && maTV);
    ThanhVien *cur = da->danhSachTV;
    while (cur) {
        if (strcmp(cur->maTV, maTV) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Tim TV theo maTV, chi tra ve node Active, Dung cho validate nhap moi */
ThanhVien *timThanhVienTrongDuAn(DuAn *da, const char *maTV) {
    assert(da && maTV);
    ThanhVien *cur = da->danhSachTV;
    while (cur) {
        if (strcmp(cur->maTV, maTV) == 0 && cur->trangThai == 1)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

void themThanhVienVaoDuAn(DuAn *da, ThanhVien *tv) {
    assert(da && tv);
    ThanhVien *existing = timTVKeCaGomXoa(da, tv->maTV);
    if (existing) {
        if (existing->trangThai == 0) {
            /* Kich hoat lai TV da Delete */
            existing->trangThai = 1;
            strncpy(existing->hoTen,  tv->hoTen,  MAX_NAME-1);
            strncpy(existing->vaiTro, tv->vaiTro, MAX_ROLE-1);
            free(tv);
            printf("  Da kich hoat lai thanh vien '%s'.\n", existing->maTV);
        } else {
            printf("  Thanh vien '%s' da co trong du an!\n", tv->maTV);
            free(tv);
        }
        return;
    }
    tv->next = NULL;
    if (!da->danhSachTV) { da->danhSachTV = tv; return; }
    ThanhVien *cur = da->danhSachTV;
    while (cur->next) cur = cur->next;
    cur->next = tv;
}

/* Dat trangThai = 0 — KHONG free() de bao toan Timesheet lich su */
void xoaThanhVienKhoiDuAn(DuAn *da, const char *maTV) {
    assert(da && maTV);
    ThanhVien *cur = da->danhSachTV;
    while (cur) {
        if (strcmp(cur->maTV, maTV) == 0) {
            if (cur->trangThai == 0) {
                printf("  Thanh vien '%s' da duoc vo hieu hoa truoc do.\n", maTV);
                return;
            }
            cur->trangThai = 0;
            printf("  Da vo hieu hoa thanh vien '%s'.\n", maTV);
            printf("  Du lieu Timesheet lich su van duoc bao toan.\n");
            return;
        }
        cur = cur->next;
    }
    printf("  Khong tim thay thanh vien '%s'.\n", maTV);
}

void giaiPhongDanhSachTV(ThanhVien *head) {
    while (head) { ThanhVien *t = head->next; free(head); head = t; }
}


DuAn *taoDuAn(const char *ma, const char *ten,
              const char *batDau, const char *ketThuc,
              const char *kh, float ns) {
    assert(ma && ten);
    DuAn *da = (DuAn *)malloc(sizeof(DuAn));
    if (!da) { fprintf(stderr, "Loi bo nho!\n"); return NULL; }
    strncpy(da->maDuAn,      ma,      MAX_ID-1);   da->maDuAn[MAX_ID-1]      = '\0';
    strncpy(da->tenDuAn,     ten,     MAX_NAME-1);  da->tenDuAn[MAX_NAME-1]   = '\0';
    strncpy(da->ngayBatDau,  batDau,  MAX_DATE-1);  da->ngayBatDau[MAX_DATE-1] = '\0';
    strncpy(da->ngayKetThuc, ketThuc, MAX_DATE-1);  da->ngayKetThuc[MAX_DATE-1]= '\0';
    strncpy(da->khachHang,   kh,      MAX_NAME-1);  da->khachHang[MAX_NAME-1]  = '\0';
    da->nganSach   = (ns > 0) ? ns : 0.0f;
    da->danhSachTV = NULL;
    da->next       = NULL;
    return da;
}

/* tim kiem tuyen tinh theo maDuAn (chinh xac) */
DuAn *timDuAnTheoMa(HeThong *ht, const char *maDuAn) {
    assert(ht && maDuAn);
    DuAn *cur = ht->headDuAn;
    while (cur) {
        if (strcmp(cur->maDuAn, maDuAn) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

void themDuAnVaoDS(HeThong *ht, DuAn *da) {
    assert(ht && da);
    if (timDuAnTheoMa(ht, da->maDuAn)) {
        printf("  Ma du an '%s' da ton tai!\n", da->maDuAn);
        return;
    }
    da->next = NULL;
    if (!ht->headDuAn) { ht->headDuAn = da; return; }
    DuAn *cur = ht->headDuAn;
    while (cur->next) cur = cur->next;
    cur->next = da;
}

/* xoa du an va toan bo thanh vien cua no */
void xoaDuAnKhoiDS(HeThong *ht, const char *maDuAn) {
    assert(ht && maDuAn);
    DuAn *cur = ht->headDuAn, *prev = NULL;
    while (cur) {
        if (strcmp(cur->maDuAn, maDuAn) == 0) {
            if (prev) prev->next = cur->next;
            else       ht->headDuAn = cur->next;
            giaiPhongDanhSachTV(cur->danhSachTV);
            free(cur);
            printf("  Da xoa du an '%s'.\n", maDuAn);
            return;
        }
        prev = cur; cur = cur->next;
    }
    printf("  Khong tim thay du an '%s'.\n", maDuAn);
}

void giaiPhongDanhSachDA(DuAn *head) {
    while (head) {
        DuAn *t = head->next;
        giaiPhongDanhSachTV(head->danhSachTV);
        free(head);
        head = t;
    }
}


void inThongTinDuAn(const DuAn *da) {
    if (!da) return;
    printf("  Ma       : %s\n",  da->maDuAn);
    printf("  Ten      : %s\n",  da->tenDuAn);
    printf("  Thoi gian: %s  ->  %s\n", da->ngayBatDau, da->ngayKetThuc);
    printf("  KH       : %s\n",  da->khachHang);
    printf("  Ngan sach: %.0f VND\n", da->nganSach);
    printf("  Thanh vien (dang hoat dong):\n"); /* chi hien thi TV Active */
    ThanhVien *tv = da->danhSachTV;
    int dem = 0;
    while (tv) {
        if (tv->trangThai == 1) {
            printf("    - [%s] %s (%s)\n", tv->maTV, tv->hoTen, tv->vaiTro);
            dem++;
        }
        tv = tv->next;
    }
    if (dem == 0) printf("    (Khong co thanh vien nao dang hoat dong)\n");
    inDuongKe(60);
}

void inDanhSachDuAn(const HeThong *ht) {
    assert(ht);
    printf("\n  === DANH SACH DU AN ===\n");
    inDuongKe(60);
    DuAn *cur = ht->headDuAn;
    if (!cur) { printf("  (Chua co du an nao)\n"); return; }
    int stt = 1;
    while (cur) { printf("  [%d] Thong Tin Du An\n", stt++); inThongTinDuAn(cur); cur = cur->next; }
}


static void nhapThemDuAn(HeThong *ht) {
    char ma[MAX_ID], ten[MAX_NAME], bd[MAX_DATE], kt[MAX_DATE];
    char kh[MAX_NAME], nsBuf[20];
    float ns;

    printf("\n  --- THEM DU AN MOI ---\n");
   do {
    printf("  Ma du an: ");
    docChuoi(ma, MAX_ID);
    if (!ma[0]) continue;              
    if (timDuAnTheoMa(ht, ma)) {       /* kiem tra trung     */
        printf("  [!] Ma du an '%s' da ton tai!"
               " Vui long nhap ma khac.\n", ma);
        ma[0] = '\0';                  /* reset de lap lai   */
    }
} while (!ma[0]);
    
    do { printf("  Ten du an: "); docChuoi(ten, MAX_NAME); } while (!ten[0]);

    do {
        do {
            printf("  Ngay bat dau (DD/MM/YYYY): ");
            docChuoi(bd, MAX_DATE);
            if (!hopLeNgay(bd)) printf("  Ngay khong hop le.\n");
        } while (!hopLeNgay(bd));
        do {
            printf("  Ngay ket thuc (DD/MM/YYYY): ");
            docChuoi(kt, MAX_DATE);
            if (!hopLeNgay(kt)) printf("  Ngay khong hop le.\n");
        } while (!hopLeNgay(kt));
        if (chuyenNgayThanhSo(kt) < chuyenNgayThanhSo(bd))
            printf("  Ngay ket thuc phai sau ngay bat dau! Nhap lai ca hai.\n");
        else break;
    } while (1);

    do { printf("  Khach hang: "); docChuoi(kh, MAX_NAME); } while (!kh[0]);
    do {
        printf("  Ngan sach (VND, > 0): ");
        docChuoi(nsBuf, sizeof(nsBuf));
        ns = atof(nsBuf);
        if (ns <= 0) printf("  Ngan sach phai > 0.\n");
    } while (ns <= 0);

    DuAn *da = taoDuAn(ma, ten, bd, kt, kh, ns);
    if (da) themDuAnVaoDS(ht, da);
    printf("  Da them du an '%s' thanh cong.\n", da->tenDuAn);
}

static void suaDuAn(HeThong *ht) {
    char ma[MAX_ID], buf[MAX_NAME];
    printf("\n  --- SUA DU AN ---\n");
    printf("  Ma du an can sua: "); docChuoi(ma, MAX_ID);
    DuAn *da = timDuAnTheoMa(ht, ma);
    if (!da) { printf("  Khong tim thay '%s'.\n", ma); return; }
    inThongTinDuAn(da);
    printf("  (Enter = giu nguyen)\n");

    printf("  Ten moi [%s]: ", da->tenDuAn);
    docChuoi(buf, MAX_NAME);
    if (buf[0]) strncpy(da->tenDuAn, buf, MAX_NAME-1);

    char bdTam[MAX_DATE], ktTam[MAX_DATE];
    strcpy(bdTam, da->ngayBatDau);
    strcpy(ktTam, da->ngayKetThuc);
    do {
        printf("  Ngay bat dau [%s]: ", da->ngayBatDau);
        docChuoi(buf, MAX_DATE);
        if (buf[0]) {
            if (!hopLeNgay(buf)) { printf("  Dinh dang sai.\n"); continue; }
            strcpy(bdTam, buf);
        }
        printf("  Ngay ket thuc [%s]: ", da->ngayKetThuc);
        docChuoi(buf, MAX_DATE);
        if (buf[0]) {
            if (!hopLeNgay(buf)) { printf("  Dinh dang sai.\n"); continue; }
            strcpy(ktTam, buf);
        }
        if (chuyenNgayThanhSo(ktTam) < chuyenNgayThanhSo(bdTam))
            printf("  Ngay ket thuc khong duoc truoc ngay bat dau.\n");
        else { strcpy(da->ngayBatDau, bdTam); strcpy(da->ngayKetThuc, ktTam); break; }
    } while (1);

    printf("  Khach hang [%s]: ", da->khachHang);
    docChuoi(buf, MAX_NAME);
    if (buf[0]) strncpy(da->khachHang, buf, MAX_NAME-1);

    printf("  Ngan sach [%.0f] (0=giu): ", da->nganSach);
    docChuoi(buf, sizeof(buf));
    float ns = atof(buf);
    if (ns > 0) da->nganSach = ns;
    printf("  Cap nhat thanh cong.\n");
}

static void themThanhVienUI(HeThong *ht) {
    char maDuAn[MAX_ID], maTV[MAX_ID], hoTen[MAX_NAME], vaiTro[MAX_ROLE];
    printf("\n  --- THEM THANH VIEN ---\n");
    printf("  Ma du an: "); docChuoi(maDuAn, MAX_ID);
    DuAn *da = timDuAnTheoMa(ht, maDuAn);
    if (!da) { printf("  Khong tim thay du an.\n"); return; }
    do { printf("  Ma thanh vien: "); docChuoi(maTV, MAX_ID); } while (!maTV[0]);
    do { printf("  Ho ten: "); docChuoi(hoTen, MAX_NAME); } while (!hoTen[0]);
    do {
        printf("  Vai tro (BA/Dev/Test/Design): ");
        docChuoi(vaiTro, MAX_ROLE);
        if (!hopLeVaiTro(vaiTro)) printf("  Vai tro khong hop le.\n");
    } while (!hopLeVaiTro(vaiTro));
    ThanhVien *tv = taoThanhVien(maTV, hoTen, vaiTro);
    if (tv) themThanhVienVaoDuAn(da, tv);
}

static void xoaThanhVienUI(HeThong *ht) {
    char maDuAn[MAX_ID], maTV[MAX_ID];
    printf("\n  --- VO HIEU HOA THANH VIEN ---\n");
    printf("  Ma du an: "); docChuoi(maDuAn, MAX_ID);
    DuAn *da = timDuAnTheoMa(ht, maDuAn);
    if (!da) { printf("  Khong tim thay du an.\n"); return; }
    printf("  Ma thanh vien: "); docChuoi(maTV, MAX_ID);
    xoaThanhVienKhoiDuAn(da, maTV);
}

static void xoaDuAnUI(HeThong *ht) {
    char ma[MAX_ID], xn[5];
    printf("\n  --- XOA DU AN ---\n");
    printf("  Ma du an: "); docChuoi(ma, MAX_ID);
    DuAn *da = timDuAnTheoMa(ht, ma);
    if (!da) { printf("  Khong tim thay '%s'.\n", ma); return; }
    printf("  Xac nhan xoa '%s'? (y/n): ", da->tenDuAn);
    docChuoi(xn, sizeof(xn));
    if (strcmp(xn,"y")==0 || strcmp(xn,"Y")==0) xoaDuAnKhoiDS(ht, ma);
    else printf("  Da huy.\n");
}

static void timKiemUI(HeThong *ht) {
    char buf[MAX_NAME];
    printf("\n  1. Tim theo ma   2. Tim theo ten\n  Lua chon: ");
    docChuoi(buf, sizeof(buf));
    if (atoi(buf) == 1) {
        printf("  Ma du an: "); docChuoi(buf, MAX_ID);
        DuAn *da = timDuAnTheoMa(ht, buf);
        if (da) inThongTinDuAn(da); else printf("  Khong tim thay.\n");
    } else {
        printf("  Tu khoa: "); docChuoi(buf, MAX_NAME);
        DuAn *cur = ht->headDuAn; int dem = 0;
        while (cur) {
            if (strstr(cur->tenDuAn, buf)) { inThongTinDuAn(cur); dem++; }
            cur = cur->next;
        }
        printf("  Tim thay %d du an.\n", dem);
    }
}

void menuQuanLyDuAn(HeThong *ht) {
    char buf[10]; int chon;
    do {
        printf("\n"); inDuongKe(50);
        printf("       QUAN LY DU AN\n"); inDuongKe(50);
        printf("  1. Xem danh sach\n  2. Them du an\n  3. Sua du an\n");
        printf("  4. Xoa du an\n  5. Tim kiem\n");
        printf("  6. Them thanh vien\n  7. Vo hieu hoa thanh vien\n");
        printf("  0. Quay lai\n"); inDuongKe(50);
        printf("  Lua chon: ");
        if (!docChuoi(buf, sizeof(buf))) { chon=-1; continue; }
        chon = atoi(buf);
        switch (chon) {
            case 1: inDanhSachDuAn(ht);   break;
            case 2: nhapThemDuAn(ht);     break;
            case 3: suaDuAn(ht);          break;
            case 4: xoaDuAnUI(ht);        break;
            case 5: timKiemUI(ht);        break;
            case 6: themThanhVienUI(ht);  break;
            case 7: xoaThanhVienUI(ht);   break;
            case 0: break;
            default: printf("  Lua chon khong hop le.\n");
        }
    } while (chon != 0);
}


/* ============================================================
   PH?N 4: TIMESHEET
   ============================================================ */

Timesheet *taoTimesheet(const char *maCV, const char *tenCV,
                        const char *loai, const char *maTV,
                        const char *maDuAn, const char *ngay,
                        float soGio, const char *trangThai) {
    assert(maCV && tenCV && loai && maTV && maDuAn && ngay && trangThai);
    Timesheet *ts = (Timesheet *)malloc(sizeof(Timesheet));
    if (!ts) { fprintf(stderr, "Loi bo nho!\n"); return NULL; }
    strncpy(ts->maCongViec,  maCV,      MAX_ID-1);    ts->maCongViec[MAX_ID-1]    = '\0';
    strncpy(ts->tenCongViec, tenCV,     MAX_NAME-1);   ts->tenCongViec[MAX_NAME-1] = '\0';
    strncpy(ts->loaiHinh,    loai,      MAX_ROLE-1);   ts->loaiHinh[MAX_ROLE-1]   = '\0';
    strncpy(ts->maTV,        maTV,      MAX_ID-1);     ts->maTV[MAX_ID-1]         = '\0';
    strncpy(ts->maDuAn,      maDuAn,    MAX_ID-1);     ts->maDuAn[MAX_ID-1]       = '\0';
    strncpy(ts->ngayLam,     ngay,      MAX_DATE-1);   ts->ngayLam[MAX_DATE-1]    = '\0';
    strncpy(ts->trangThai,   trangThai, MAX_STATUS-1); ts->trangThai[MAX_STATUS-1]= '\0';
    ts->soGio = (soGio > 0) ? soGio : 0.0f;
    ts->next  = NULL;
    return ts;
}

/* tim kiem timesheet theo maCongViec */
static Timesheet *timTimesheetTheoMa(HeThong *ht, const char *maCV) {
    assert(ht && maCV);
    Timesheet *cur = ht->headTS;
    while (cur) {
        if (strcmp(cur->maCongViec, maCV) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

void themTimesheetVaoDS(HeThong *ht, Timesheet *ts) {
    assert(ht && ts);
    if (timTimesheetTheoMa(ht, ts->maCongViec)) {
        printf("  Ma cong viec '%s' da ton tai!\n", ts->maCongViec);
        free(ts);
        return;
    }
    ts->next = NULL;
    if (!ht->headTS) { ht->headTS = ts; return; }
    Timesheet *cur = ht->headTS;
    while (cur->next) cur = cur->next;
    cur->next = ts;
}

void xoaTimesheetKhoiDS(HeThong *ht, const char *maCV) {
    assert(ht && maCV);
    Timesheet *cur = ht->headTS, *prev = NULL;
    while (cur) {
        if (strcmp(cur->maCongViec, maCV) == 0) {
            if (prev) prev->next = cur->next; else ht->headTS = cur->next;
            free(cur);
            printf("  Da xoa '%s'.\n", maCV);
            return;
        }
        prev = cur; cur = cur->next;
    }
    printf("  Khong tim thay '%s'.\n", maCV);
}

void giaiPhongDanhSachTS(Timesheet *head) {
    while (head) { Timesheet *t = head->next; free(head); head = t; }
}

void inDanhSachTimesheet(const HeThong *ht) {
    assert(ht);
    printf("\n  === DANH SACH TIMESHEET ===\n");
    inDuongKe(75);
    printf("  %-10s %-22s %-8s %-8s %-10s %-6s %-12s\n",
           "MaCV","TenCV","Loai","MaNV","MaDuAn","Gio","TrangThai");
    inDuongKe(75);
    Timesheet *cur = ht->headTS;
    if (!cur) { printf("  (Chua co du lieu)\n"); return; }
    while (cur) {
        printf("  %-10s %-22s %-8s %-8s %-10s %-6.1f %-12s\n",
               cur->maCongViec, cur->tenCongViec, cur->loaiHinh,
               cur->maTV, cur->maDuAn, cur->soGio, cur->trangThai);
        cur = cur->next;
    }
}

static void nhapThemTimesheet(HeThong *ht) {
    char maCV[MAX_ID], tenCV[MAX_NAME], loai[MAX_ROLE];
    char maTV[MAX_ID], maDuAn[MAX_ID], ngay[MAX_DATE];
    char trangThai[MAX_STATUS], buf[20];
    float soGio;

    printf("\n  --- THEM TIMESHEET ---\n");
    do { printf("  Ma cong viec: "); docChuoi(maCV, MAX_ID); } while (!maCV[0]);

    if (timTimesheetTheoMa(ht, maCV)) {
        printf("  Ma cong viec '%s' da ton tai.\n", maCV); return;
    }

    do { printf("  Ten cong viec: "); docChuoi(tenCV, MAX_NAME); } while (!tenCV[0]);
    do {
        printf("  Loai hinh (BA/Dev/Test/Design): ");
        docChuoi(loai, MAX_ROLE);
        if (!hopLeVaiTro(loai)) printf("  Loai hinh khong hop le.\n");
    } while (!hopLeVaiTro(loai));

    DuAn *da = NULL;
    do {
        printf("  Ma du an: "); docChuoi(maDuAn, MAX_ID);
        if (!maDuAn[0]) continue;
        da = timDuAnTheoMa(ht, maDuAn);
        if (!da) printf("  Du an '%s' khong ton tai.\n", maDuAn);
    } while (!da);
    
    int coTVActive = 0;
    ThanhVien *tvCheck = da->danhSachTV;
    while (tvCheck) {
        if (tvCheck->trangThai == 1) { coTVActive = 1; break; }
        tvCheck = tvCheck->next;
    }
    if (!coTVActive) {
        printf("  Du an '%s' chua co thanh vien nao.\n", maDuAn);
        printf("  Vui long them thanh vien truoc khi nhap Timesheet.\n");
        return;
    }

/* Validate maTV có thuoc du an do khong */
    do {
        printf("  Ma nhan vien: "); docChuoi(maTV, MAX_ID);
        if (!maTV[0]) continue;
        if (!timThanhVienTrongDuAn(da, maTV))
            printf("  Nhan vien '%s' khong thuoc du an '%s'.\n", maTV, maDuAn);
    } while (!maTV[0] || !timThanhVienTrongDuAn(da, maTV));

    do {
        printf("  Ngay lam (DD/MM/YYYY): ");
        docChuoi(ngay, MAX_DATE);
        if (!hopLeNgay(ngay)) {
            printf("  Ngay khong hop le.\n");
            continue;
        }
        long soNgay = chuyenNgayThanhSo(ngay);
        long soBD   = chuyenNgayThanhSo(da->ngayBatDau);
        long soKT   = chuyenNgayThanhSo(da->ngayKetThuc);
        if (soNgay < soBD || soNgay > soKT)
            printf("  Ngay lam phai trong khoang du an: %s -> %s\n",
                   da->ngayBatDau, da->ngayKetThuc);
        else
            break;
    } while (1);
    do {
        printf("  So gio (> 0): "); docChuoi(buf, sizeof(buf));
        soGio = atof(buf);
        if (soGio <= 0) printf("  So gio phai > 0.\n");
    } while (soGio <= 0);
    do {
        printf("  Trang thai (Done/In Progress): ");
        docChuoi(trangThai, MAX_STATUS);
        if (!hopLeTrangThai(trangThai)) printf("  Trang thai khong hop le.\n");
    } while (!hopLeTrangThai(trangThai));

    Timesheet *ts = taoTimesheet(maCV, tenCV, loai, maTV, maDuAn,
                                  ngay, soGio, trangThai);
    if (ts) { themTimesheetVaoDS(ht, ts); printf("  Them thanh cong.\n"); }
}

static void suaTimesheetUI(HeThong *ht) {
    char maCV[MAX_ID], buf[MAX_NAME];
    printf("\n  --- SUA TIMESHEET ---\n");
    printf("  Ma cong viec: "); docChuoi(maCV, MAX_ID);
    Timesheet *cur = ht->headTS;
    while (cur && strcmp(cur->maCongViec, maCV) != 0) cur = cur->next;
    if (!cur) { printf("  Khong tim thay.\n"); return; }
    printf("  Ten moi [%s]: ", cur->tenCongViec);
    docChuoi(buf, MAX_NAME);
    if (buf[0]) strncpy(cur->tenCongViec, buf, MAX_NAME-1);
    do {
        printf("  Trang thai [%s]: ", cur->trangThai);
        docChuoi(buf, MAX_STATUS);
        if (!buf[0]) break;
        if (!hopLeTrangThai(buf)) printf("  Khong hop le.\n");
        else { strncpy(cur->trangThai, buf, MAX_STATUS-1); break; }
    } while (1);
    printf("  So gio [%.1f] (0=giu): ", cur->soGio);
    docChuoi(buf, sizeof(buf));
    float g = atof(buf);
    if (g > 0) cur->soGio = g;
    printf("  Cap nhat thanh cong.\n");
}

void menuTimesheet(HeThong *ht) {
    char buf[10]; int chon;
    do {
        printf("\n"); inDuongKe(50);
        printf("       QUAN LY TIMESHEET\n"); inDuongKe(50);
        printf("  1. Xem tat ca\n  2. Them moi\n  3. Sua\n  4. Xoa\n  0. Quay lai\n");
        inDuongKe(50); printf("  Lua chon: ");
        if (!docChuoi(buf, sizeof(buf))) { chon=-1; continue; }
        chon = atoi(buf);
        char maCV[MAX_ID];
        switch (chon) {
            case 1: inDanhSachTimesheet(ht); break;
            case 2: nhapThemTimesheet(ht);   break;
            case 3: suaTimesheetUI(ht);      break;
            case 4:
                printf("  Ma cong viec: "); docChuoi(maCV, MAX_ID);
                xoaTimesheetKhoiDS(ht, maCV); break;
            case 0: break;
            default: printf("  Lua chon khong hop le.\n");
        }
    } while (chon != 0);
}


/* ============================================================
   PHAN 5: CHI PHI & BAO CAO
   ============================================================ */

/* duyet toan bo timesheet thuoc maDuAn, nhan soGio * donGia(vaiTro) roi cong don */
float tinhTongChiPhi(const HeThong *ht, const char *maDuAn) {
    assert(ht && maDuAn);
    float tong = 0.0f;
    Timesheet *cur = ht->headTS;
    while (cur) {
        if (strcmp(cur->maDuAn, maDuAn) == 0)
            tong += cur->soGio * donGiaVaiTro(cur->loaiHinh);
        cur = cur->next;
    }
    return tong;
}

/* Duyet Timesheet 1 LAN, tra ve tong de tai su dung */
float inChiPhiChiTiet(const HeThong *ht, const char *maDuAn) {
    assert(ht && maDuAn);
    float gioBA=0, gioDev=0, gioTest=0, gioDesign=0;
    int   demBA=0, demDev=0, demTest=0, demDesign=0;
    Timesheet *cur = ht->headTS;
    while (cur) {
        if (strcmp(cur->maDuAn, maDuAn) == 0) {
            if      (strcmp(cur->loaiHinh,"BA")     == 0) { gioBA     += cur->soGio; demBA++;     }
            else if (strcmp(cur->loaiHinh,"Dev")    == 0) { gioDev    += cur->soGio; demDev++;    }
            else if (strcmp(cur->loaiHinh,"Test")   == 0) { gioTest   += cur->soGio; demTest++;   }
            else if (strcmp(cur->loaiHinh,"Design") == 0) { gioDesign += cur->soGio; demDesign++; }
        }
        cur = cur->next;
    }
    printf("\n  %-10s %-8s %-12s %-12s %-15s\n",
           "Vai tro","So CV","Tong gio","Don gia/h","Thanh tien");
    inDuongKe(65);
    printf("  %-10s %-8d %-12.1f %-12.0f %-15.0f\n","BA",     demBA,     gioBA,     GIA_BA,     gioBA    *GIA_BA);
    printf("  %-10s %-8d %-12.1f %-12.0f %-15.0f\n","Dev",    demDev,    gioDev,    GIA_DEV,    gioDev   *GIA_DEV);
    printf("  %-10s %-8d %-12.1f %-12.0f %-15.0f\n","Test",   demTest,   gioTest,   GIA_TEST,   gioTest  *GIA_TEST);
    printf("  %-10s %-8d %-12.1f %-12.0f %-15.0f\n","Design", demDesign, gioDesign, GIA_DESIGN, gioDesign*GIA_DESIGN);
    inDuongKe(65);
    float tong = gioBA*GIA_BA + gioDev*GIA_DEV + gioTest*GIA_TEST + gioDesign*GIA_DESIGN;
    printf("  TONG CHI PHI NHAN CONG: %.0f VND\n", tong);
    return tong;
}

void soSanhNganSachVoiChiPhi(const HeThong *ht, const char *maDuAn, float chiPhi) {
    assert(ht && maDuAn);
    DuAn *da = timDuAnTheoMa((HeThong *)ht, maDuAn);
    if (!da) { printf("  Khong tim thay du an.\n"); return; }
    float ns    = da->nganSach;
    float chenh = chiPhi - ns;
    float pct   = (ns > 0) ? (chiPhi / ns * 100.0f) : 0.0f;
    printf("\n  === SO SANH NGAN SACH - %s ===\n", da->tenDuAn);
    inDuongKe(55);
    printf("  Ngan sach du kien  : %15.0f VND\n", ns);
    printf("  Chi phi thuc te    : %15.0f VND\n", chiPhi);
    if (ns <= 0) { printf("  Ngan sach chua thiet lap.\n"); return; }
    printf("  Da su dung         : %14.1f%%\n", pct);
    if      (chenh > 0)    printf("  *** CANH BAO: VUOT NGAN SACH %.0f VND (%.1f%%) ***\n", chenh, chenh/ns*100);
    else if (pct >= 80.0f) printf("  Can than: Da dung %.1f%%. Con lai: %.0f VND\n", pct, -chenh);
    else                   printf("  Con lai: %.0f VND (%.1f%% chua dung)\n", -chenh, 100.0f-pct);
    inDuongKe(55);
}

void soSanhNganSach(const HeThong *ht, const char *maDuAn) {
    float chiPhi = tinhTongChiPhi(ht, maDuAn);
    soSanhNganSachVoiChiPhi(ht, maDuAn, chiPhi);
}

void baoCaoTongHop(const HeThong *ht) {
    assert(ht);
    printf("\n"); inDuongKe(70);
    printf("                  BAO CAO TONG HOP HE THONG\n"); inDuongKe(70);
    DuAn *da = ht->headDuAn;
    if (!da) { printf("  (Chua co du an nao)\n"); return; }
    float tongToanBo = 0.0f;
    int   soDuAn     = 0;
    while (da) {
        soDuAn++;
        printf("\n  [%d] Du an: %s (%s)\n", soDuAn, da->tenDuAn, da->maDuAn);
        int done=0, inprog=0;
        Timesheet *ts = ht->headTS;
        while (ts) {
            if (strcmp(ts->maDuAn, da->maDuAn) == 0) {
                if (strcmp(ts->trangThai,"Done")==0) done++; else inprog++;
            }
            ts = ts->next;
        }
        int tongCV = done + inprog;
        printf("  Cong viec: %d  |  Done: %d  |  In Progress: %d\n",
               tongCV, done, inprog);
        if (tongCV > 0)
            printf("  Tien do  : %.1f%%\n", (float)done/tongCV*100.0f);
        float chiPhiDA = inChiPhiChiTiet(ht, da->maDuAn);
        soSanhNganSachVoiChiPhi(ht, da->maDuAn, chiPhiDA);
        tongToanBo += chiPhiDA;
        da = da->next;
    }
    inDuongKe(70);
    printf("  TONG CHI PHI TOAN BO: %.0f VND\n", tongToanBo);
    printf("  TONG SO DU AN       : %d\n", soDuAn);
    inDuongKe(70);
}

void menuChiPhi(HeThong *ht) {
    char buf[MAX_ID]; int chon;
    do {
        printf("\n"); inDuongKe(50);
        printf("       CHI PHI & BAO CAO\n"); inDuongKe(50);
        printf("  1. Chi phi mot du an\n  2. So sanh ngan sach\n");
        printf("  3. Bao cao tong hop\n  0. Quay lai\n");
        inDuongKe(50); printf("  Lua chon: ");
        if (!docChuoi(buf, sizeof(buf))) { chon=-1; continue; }
        chon = atoi(buf);
        char maDuAn[MAX_ID];
        switch (chon) {
            case 1: printf("  Ma du an: "); docChuoi(maDuAn, MAX_ID); inChiPhiChiTiet(ht, maDuAn); break;
            case 2: printf("  Ma du an: "); docChuoi(maDuAn, MAX_ID); soSanhNganSach(ht, maDuAn);  break;
            case 3: baoCaoTongHop(ht); break;
            case 0: break;
            default: printf("  Lua chon khong hop le.\n");
        }
    } while (chon != 0);
}


/* ============================================================
   PHAN 6: DOC / GHI FILE
   ============================================================ */

/* Tach chuoi theo dau '|', tra ve so token */
static int tachToken(char *src, char *tokens[], int maxTok) {
    int n = 0;
    char *p = src;
    tokens[n++] = p;
    while (*p && n < maxTok) {
        if (*p == '|') { *p = '\0'; tokens[n++] = p+1; }
        p++;
    }
    int last = n-1, len = strlen(tokens[last]);
    if (len > 0 && tokens[last][len-1] == '\n') tokens[last][len-1] = '\0';
    return n;
}

/* ghi tat ca du lieu ra 3 file */
void luuDuLieu(const HeThong *ht) {
    assert(ht);
    FILE *f;

    f = fopen(FILE_DUAN, "w");
    if (!f) { fprintf(stderr, "Loi mo file %s\n", FILE_DUAN); return; }
    DuAn *da = ht->headDuAn;
    while (da) {
        fprintf(f, "%s|%s|%s|%s|%s|%.0f\n",
                da->maDuAn, da->tenDuAn, da->ngayBatDau,
                da->ngayKetThuc, da->khachHang, da->nganSach);
        da = da->next;
    }
    fclose(f);

    f = fopen(FILE_THANHVIEN, "w");
    if (!f) { fprintf(stderr, "Loi mo file %s\n", FILE_THANHVIEN); return; }
    da = ht->headDuAn;
    while (da) {
        ThanhVien *tv = da->danhSachTV;
        while (tv) {
            fprintf(f, "%s|%s|%s|%s|%d\n",           /* Format: MASV | HOTEN | VAITRO | MADUAN | TRANGTHAI */
                    tv->maTV, tv->hoTen, tv->vaiTro,
                    da->maDuAn, tv->trangThai);
            tv = tv->next;
        }
        da = da->next;
    }
    fclose(f);

    f = fopen(FILE_TIMESHEET, "w");
    if (!f) { fprintf(stderr, "Loi mo file %s\n", FILE_TIMESHEET); return; }
    Timesheet *ts = ht->headTS;
    while (ts) {
        fprintf(f, "%s|%s|%s|%s|%s|%s|%.1f|%s\n",
                ts->maCongViec, ts->tenCongViec, ts->loaiHinh,
                ts->maTV, ts->maDuAn, ts->ngayLam, ts->soGio, ts->trangThai);
        ts = ts->next;
    }
    fclose(f);
    printf("  Da luu toan bo du lieu vao file text.\n");
}

/* doc tu 3 file, nap vào HeThong */
void docDuLieu(HeThong *ht) {
    assert(ht);
    FILE *f;
    char dong[256];
    char *tokens[10];

    /* 1. Doc Du An */
    f = fopen(FILE_DUAN, "r");
    if (f) {
        while (fgets(dong, sizeof(dong), f)) {
            int n = tachToken(dong, tokens, 6);
            if (n >= 6) {
                DuAn *da = taoDuAn(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], atof(tokens[5]));
                if (da) themDuAnVaoDS(ht, da);
            }
        }
        fclose(f);
    }

    /* 2. Doc Thanh Vien */
    f = fopen(FILE_THANHVIEN, "r");
    if (f) {
        while (fgets(dong, sizeof(dong), f)) {
            int n = tachToken(dong, tokens, 5);
            if (n >= 5) {
                DuAn *da = timDuAnTheoMa(ht, tokens[3]);
                if (da) {
                    ThanhVien *tv = taoThanhVien(tokens[0], tokens[1], tokens[2]);
                    if (tv) {
                        tv->trangThai = atoi(tokens[4]);
                        /* Them truc tiep vao DSLK duoi du an */
                        tv->next = NULL;
                        if (!da->danhSachTV) {
                            da->danhSachTV = tv;
                        } else {
                            ThanhVien *cur = da->danhSachTV;
                            while (cur->next) cur = cur->next;
                            cur->next = tv;
                        }
                    }
                }
            }
        }
        fclose(f);
    }

    /* 3. Doc Timesheet */
    f = fopen(FILE_TIMESHEET, "r");
    if (f) {
        while (fgets(dong, sizeof(dong), f)) {
            int n = tachToken(dong, tokens, 8);
            if (n >= 8) {
                Timesheet *ts = taoTimesheet(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5], atof(tokens[6]), tokens[7]);
                if (ts) themTimesheetVaoDS(ht, ts);
            }
        }
        fclose(f);
    }
}


/* ============================================================
   PHAN 7: MAIN HAM DIEU KHIEN CHINH
   ============================================================ */

int main(void) {
    HeThong ht = {NULL, NULL, NULL};
    char buf[10];
    int chon;

    /* Tu dong load database cu len khi chay chuong trinh */
    docDuLieu(&ht);

    do {
        printf("\n"); inDuongKe(55);
        printf("   HE THONG QUAN LY CONG VIEC DU AN \n");
        inDuongKe(55);
        printf("  1. Chuc nang Quan ly Du an & Thanh vien\n");
        printf("  2. Chuc nang Quan ly Timesheet \n");
        printf("  3. Xem Chi phi & Bao cao Thong ke\n");
        printf("  4. Luu du lieu vao File\n");
        printf("  0. Thoat chuong trinh\n");
        inDuongKe(55);
        printf("  Lua chon cua ban: ");
        
        if (!docChuoi(buf, sizeof(buf))) { chon = -1; continue; }
        chon = atoi(buf);

        switch (chon) {
            case 1:
                menuQuanLyDuAn(&ht);
                break;
            case 2:
                menuTimesheet(&ht);
                break;
            case 3:
                menuChiPhi(&ht);
                break;
            case 4:
                luuDuLieu(&ht);
                break;
            case 0:
                /* Tu dong luu lai truoc khi thoat han */
                luuDuLieu(&ht);
                giaiPhongDanhSachDA(ht.headDuAn);
                giaiPhongDanhSachTS(ht.headTS);
                printf("\n  Da giai phong bo nho. Tam biet!\n");
                break;
            default:
                printf("  Lua chon khong hop le. Vui long chon lai.\n");
        }
    } while (chon != 0);

    return 0;
}
