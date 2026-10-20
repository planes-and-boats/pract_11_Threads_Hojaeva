#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <string>
#include <fstream>
#include <exception>

using namespace std;

int* testStep = nullptr;
int totalSteps = 0;

enum Status { Works, Suspended, Completed };

struct Robot {
    unsigned int id;
    int energy = 60;
    int points = 0;
    Status status = Works;
};

void Obstacle(Robot* robot) {
    Robot* rob = robot;
    if (rob->energy < 10) {
        cout << "[ОТКАЗ] Модель #" << robot->id << ": Недостаточно энергии для сектора 'Препятствие'." << endl;
        return;
    }

    Sleep(3000);

    rob->energy -= 10;
    rob->points += 15;

    cout << "[АРЕНА] Модель #" << robot->id << " | Препятствие | Энергия: " << rob->energy;
    cout << "% | Эффективность: " << rob->points << endl;
}

void Sprint(Robot* robot) {
    Robot* rob = robot;
    if (rob->energy < 20) {
        cout << "[ОТКАЗ] Модель #" << robot->id << ": Недостаточно питания для ускорения. Сектор 'Спринт' заблокирован." << endl;
        return;
    }

    Sleep(4000);

    rob->energy -= 20;
    rob->points += 30;

    cout << "[АРЕНА] Модель #" << robot->id << " | Спринт      | Энергия: " << rob->energy;
    cout << "% | Эффективность: " << rob->points << endl;
}

void Recharging(Robot* robot) {
    int time = rand() % 5 + 1;
    Sleep(time * 1000);

    Robot* rob = robot;
    rob->energy += time * 10;
    if (rob->energy > 100)
        rob->energy = 100;

    cout << "[ПИТАНИЕ] Модель #" << robot->id << " | Док-станция питания | Заряд: " << rob->energy << "%" << endl;
}

void Marathon(Robot* robot) {
    Robot* rob = robot;
    if (rob->energy < 60) {
        cout << "[ОТКАЗ] Модель #" << robot->id << ": Высокий риск деградации аккумулятора. Тест 'Марафон' отменен." << endl;
        return;
    }

    Sleep(10000);

    rob->energy -= 60;
    rob->points += 55;

    cout << "[АРЕНА] Модель #" << robot->id << " | Марафон     | Энергия: " << rob->energy;
    cout << "% | Эффективность: " << rob->points << endl;
}

void (*test[])(Robot* robot) = {
    Obstacle,
    Sprint,
    Recharging,
    Marathon
};

void applyTesting(Robot* robot, void (*test)(Robot* robot)) {
    if (test != nullptr)
        test(robot);
}

DWORD WINAPI RobotThread(LPVOID lpParam) {
    Robot* rob = (Robot*)lpParam;
    cout << "[СИСТЕМА] Модель #" << rob->id << " интегрирована в симуляцию и выведена на испытательный полигон." << endl << endl;

    for (int i = 0; i < totalSteps; i++) {
        while (rob->status == Status::Suspended) {
            Sleep(100);
        }

        void (*currentTest)(Robot* robot) = nullptr;
        currentTest = test[testStep[i] - 1];
        if (currentTest != nullptr) {
            applyTesting(rob, currentTest);
        }

        Sleep((rand() % 3 + 1) * 1000);
    }

    rob->status = Status::Completed;
    return 0;
}

int main()
{
    setlocale(NULL, "Russian");
    srand(time(NULL));

    Robot* robots = nullptr;
    HANDLE* hRobots = nullptr;
    int robotCount = 0;
    string input;

    Sleep(500);

    cout << endl << "\t--- EVE CORPORATION // ТЕСТОВЫЙ КОМПЛЕКС АРЕНА ---" << endl << endl;

    Sleep(1000);

    cout << "[СИСТЕМА] Терминал активен. Ожидание параметров тестирования моделей..." << endl << endl;

    Sleep(1000);

    try {
        ifstream in("arena.dat", ios::binary);
        if (!in.is_open()) {
            throw runtime_error("Критический сбой. Конфигурационный файл 'arena.dat' поврежден или отсутствует.");
        }

        in.seekg(0, ios::end);
        streamsize fileSize = in.tellg();
        totalSteps = fileSize / sizeof(int);
        in.seekg(0, ios::beg);

        if (totalSteps == 0) {
            throw runtime_error("База данных пуста. Сценарии испытаний не обнаружены.");
        }

        testStep = new int[totalSteps];
        in.read(reinterpret_cast<char*>(testStep), totalSteps * sizeof(int));

        do {
            cout << "[КОНСОЛЬ] Введите количество тестируемых моделей (от 3 до 10 включительно): ";
            cin >> input;
            try {
                robotCount = stoi(input);
            }
            catch (exception) {
                throw invalid_argument("Ошибка ввода. Требуется числовое значение.");
            }
        } while (robotCount < 3 || robotCount > 10);

        robots = new Robot[robotCount];
        hRobots = new HANDLE[robotCount];

        Sleep(1000);

        cout << endl << "[СИСТЕМА] Инициализация аппаратных потоков..." << endl;

        Sleep(1000);

        cout << "[СИСТЕМА] Запуск симуляции..." << endl << endl;

        for (int i = 0; i < robotCount; i++) {
            robots[i].id = i + 1;
            hRobots[i] = CreateThread(NULL, 0, RobotThread, &robots[i], 0, NULL);

            if (hRobots[i] == NULL)
                throw runtime_error("Сбой аппаратного потока для Модели #" + to_string(i + 1) + ": " + to_string(GetLastError()));

            Sleep(10);
        }

        bool allFinished = false;
        while (!allFinished) {
            allFinished = true;
            for (int i = 0; i < robotCount; i++) {
                if (robots[i].status != Status::Completed) {
                    allFinished = false;
                    break;
                }
            }

            if (allFinished) break;

            if (_kbhit()) {
                unsigned int targetId = 0;
                switch (_getch()) {
                case 'p':
                case 'P': {
                    for (int i = 0; i < robotCount; i++) {
                        SuspendThread(hRobots[i]);
                    }

                    cout << endl << "[ПЕРЕХВАТ] Введите ID модели для заморозки процессов: ";
                    cin >> targetId;
                    if (targetId >= 1 && targetId <= robotCount) {
                        robots[targetId - 1].status = Status::Suspended;
                        cout << "[СТАТУС] Модель #" << targetId << " принудительно переведена в режим гибернации." << endl << endl;
                    }

                    for (int i = 0; i < robotCount; i++) {
                        ResumeThread(hRobots[i]);
                    }
                    break;
                }
                case 'r':
                case 'R': {
                    for (int i = 0; i < robotCount; i++) {
                        SuspendThread(hRobots[i]);
                    }

                    cout << endl << "[ПЕРЕХВАТ] Введите ID модели для разморозки процессов: ";
                    cin >> targetId;
                    if (targetId >= 1 && targetId <= robotCount) {
                        robots[targetId - 1].status = Status::Works;
                        cout << "[СТАТУС] Регенерация процессов модели #" << targetId << ". Возврат на Арену." << endl << endl;
                    }

                    for (int i = 0; i < robotCount; i++) {
                        ResumeThread(hRobots[i]);
                    }
                    break;
                }

                }
            }
            Sleep(100);
        }

        WaitForMultipleObjects(robotCount, hRobots, TRUE, INFINITE);

        for (int i = 0; i < robotCount; i++) {
            CloseHandle(hRobots[i]);
        }

        for (int i = 0; i < robotCount - 1; i++) {
            for (int j = 0; j < robotCount - i - 1; j++) {
                if (robots[j].points < robots[j + 1].points) {
                    Robot temp = robots[j];
                    robots[j] = robots[j + 1];
                    robots[j + 1] = temp;
                }
            }
        }

        cout << endl << "[СИСТЕМА] Завершение симуляции." << endl << endl;

        cout << endl << " --- ИТОГОВЫЙ ОТЧЕТ ЭФФЕКТИВНОСТИ // КЛАССИФИКАЦИЯ EVE CORP ---" << endl << endl;
        cout << "\tРанг\tМодель\tОчки\tБатарея\t Статус" << endl << endl;
        for (int i = 0; i < robotCount; i++) {
            cout << "\t" << i + 1 << "\t#" << robots[i].id;
            cout << "\t" << robots[i].points << "\t" << robots[i].energy << "%\t ";
            if (robots[i].status == Status::Works) cout << "активен на Арене";
            else if (robots[i].status == Status::Suspended) cout << "режим гибернации";
            else cout << "тест завершен / списан";
            cout << endl;
        }
        cout << endl << " ---" << endl << endl;

        if (robots[0].points == robots[1].points) {
            Sleep(1000);
            cout << "[СИСТЕМА] Результаты лидеров равны." << endl;
        }

        Sleep(1000);
        cout << "[СИСТЕМА] Сессия тестирования завершена. Списание ресурсов выполнено." << endl;
        Sleep(1000);
        cout << "[СИСТЕМА] Все тестовые модели отправлены на утилизацию." << endl << endl;
        Sleep(1000);
        cout << "[УВЕДОМЛЕНИЕ] Хорошего дня, Оператор. Eve Corp следит за вашей продуктивностью." << endl << endl;
        Sleep(1000);
    }
    catch (exception& e) {
        cout << endl << "[КРИТИЧЕСКАЯ ОШИБКА]: " << e.what() << endl << endl;
    }

    delete[] testStep;
    delete[] robots;
    delete[] hRobots;

    return 0;
}