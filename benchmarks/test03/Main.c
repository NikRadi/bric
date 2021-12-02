static void PrintH() {
    printf("H");
}

static void Nothing1() { }

static void PrintI() {
    printf("i");
}

static void PrintNewLine() {
    printf("\n");
}

static void Nothing2() {
    Nothing1();
}

static void PrintHi() {
    PrintH();
    PrintI();
}

int main(int argc, char **argv) {
    PrintNewLine();
    Nothing2();
    PrintHi();
    PrintNewLine();
    return 0;
}