// ISP: узкие интерфейсы вместо одного "толстого". Демонстрация того, как
// нарушение ISP на практике перерастает в нарушение LSP - implementer
// толстого интерфейса, не умеющий что-то из него, вынужден врать (throw/
// assert в методе, который "должен" отработать по контракту типа).

#include <iostream>
#include <format>
#include <stdexcept>
#include <vector>

// ============================================================
// Вариант 1 (антипаттерн): один "толстый" интерфейс на все МФУ-подобные
// устройства.
// ============================================================
namespace bad_fat_interface {

    class IMultiFunctionPointer {
    public:
        virtual ~IMultiFunctionPointer() = default;
        virtual void print(const std::string&) = 0;
        virtual void scan(const std::string&) = 0;
        virtual void fax(const std::string&) = 0;
        virtual void staple() = 0;
    };

    // Простой принтер физически не умеет ни сканировать, ни факсить, ни
    // сшивать - но интерфейс ЗАСТАВЛЯЕТ реализовать все четыре метода.
    class BasicPrinter: public IMultiFunctionPointer {
    public:
        void print(const std::string& doc) override {
            std::cout << std::format("  print: {}\n", doc);
        }
        void scan(const std::string&) override {
            throw std::logic_error("BasicPrinter can not scan");
        }
        void fax(const std::string&) override {
            throw std::logic_error("BasicPrinter can not fax");
        }
        void staple() override {
            throw std::logic_error("BasicPrinter can not staple");
        }
    };

    // Клиентский код честно программирует ПРОТИВ интерфейса IMultiFunctionPrinter -
    // он имеет полное право вызвать scan(), раз тип это объявляет.
    static void processDocument(IMultiFunctionPointer& device, const std::string& doc) {
        device.print(doc);
        // компилируется без единого предупреждения...
        device.scan(doc);
    }

    static void run() {
        std::cout << "--- bad_fat_interface ---\n";
        BasicPrinter printer;
        try {
            processDocument(printer, "report.pdf");
        } catch (const std::exception& e) {
            std::cout << std::format("  runtime-error: {}\n", e.what());
        }
    }
}

// ============================================================
// Вариант 2: узкие интерфейсы, каждый - одна способность.
// ============================================================
namespace good_segregated_interfaces {

    class IPrinter {
    public:
        virtual ~IPrinter() = default;
        virtual void print(const std::string&) = 0;
    };

    class IScanner {
    public:
        virtual ~IScanner() = default;
        virtual void scan(const std::string&) = 0;
    };

    class IFax {
    public:
        virtual ~IFax() = default;
        virtual void fax(const std::string&) = 0;
    };

    // BasicPrinter реализует РОВНО то, что умеет - никаких лишних методов,
    // никакого вранья контракту. У него физически нет метода scan() -
    // попытка его вызвать - ошибка КОМПИЛЯЦИИ, а не рантайма.
    class BasicPrinter: public IPrinter {
    public:
        void print(const std::string& doc) override {
            std::cout << std::format("  print: {}\n", doc);
        }
    };

    // AllInOnePrinter честно комбинирует несколько узких интерфейсов через
    // множественное наследование - в C++ это дёшево и безопасно, когда базовые
    // классы чисто абстрактные (без данных - делить нечего, ромбовидной
    // проблемы наследования тут просто неоткуда взяться).
    class AllInOnePrinter: public IPrinter, public IScanner, public IFax {
    public:
        void print(const std::string& doc) override {
            std::cout << std::format("  print: {}\n", doc);
        }
        void scan(const std::string& doc) override {
            std::cout << std::format("  scan: {}\n", doc);
        }
        void fax(const std::string& doc) override {
            std::cout << std::format("  fax: {}\n", doc);
        }
    };

    // Клиентский код теперь честен по отношению к тому, что ему реально нужно -
    // эта функция физически не может вызвать scan() на объекте, у которого
    // его нет, потому что принимает только IPrinter&.
    static void printOnly(IPrinter& printer, const std::string& doc) {
        printer.print(doc);
    }

    // А эта функция явно требует и печать, и скан - видно прямо из сигнатуры,
    // какие способности нужны, без домыслов о том, "а вдруг он ещё что-то умеет".
    static void printAndScan(IPrinter& printer, IScanner& scanner, const std::string& doc) {
        printer.print(doc);
        scanner.scan(doc);
    }

    static void run() {
        std::cout << "\n--- good_segregated_interfaces ---\n";
        BasicPrinter basic;
        AllInOnePrinter allInOne;

        // ок - BasicPrinter умеет печатать
        printOnly(basic, "report.pdf");
        // ок - AllInOnePrinter тоже IPrinter
        printOnly(allInOne, "report.pdf");

        // ок - есть оба интерфейса
        printAndScan(allInOne, allInOne, "report.pdf");

        // <- НЕ СКОМПИЛИРУЕТСЯ:
        // printAndScan(allInOne, basic, "report.pdf");

        std::cout << std::format("\n  sizeof(BasicPrinter) = {} (1 vptr)\n", sizeof(BasicPrinter));
        std::cout << std::format("\n  sizeof(AllInOnePrinter) = {} (3 vptr)\n)", sizeof(AllInOnePrinter));
    }

}

int main() {
    bad_fat_interface::run();
    good_segregated_interfaces::run();

    return 0;
}
