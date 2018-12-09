#include <iostream>								// подключение библиотек ввода-вывода
#include <ctime>							    // подключение библиотек для работы с текущим временем (необходимы для генерации случайных чисел)
#include <string>								// подключение библиотек для работы со строками
#include <windows.h>							// подключение библиотек для использования кириллицы в консоли
using namespace std;							// использование пространства имен std

#define LIMIT 10000								// верхняя граница генерации числа

/*
Функция возведения числа n в степень p по модулю mod
*  https://ru.wikipedia.org/wiki/Алгоритмы_быстрого_возведения_в_степень_по_модулю
* Модификация метода повторяющихся возведения в квадрат и умножения
*/
int modulo(int n, int p, int mod)
{
	int result = 1; // результат вычисления
	for (; p; p >>= 1) // в цикле p = p / 2 
	{
		// если p нечетное, то n перемножается с результатом вычисления функции 
		if (p & 1)
			result = (1LL * result * n) % mod;
		// теперь p четное:
		n = (1LL * n * n) % mod;
	}
	return result;
}

/*
Тест Миллера-Рабина для проверки простоты числа n
* https://ru.wikipedia.org/wiki/Тест_Миллера_—_Рабина
*/
bool millerTest(int n, int d)
{
	// выбрать случайное число в диапазоне [2..n-2], n > 4 
	int a = 2 + rand() % (n - 4);

	// вычислить a^d % n 
	int x = modulo(a, d, n);

	if (x == 1 || x == n - 1)
		return true;

	// продолжать возводить в квадрат x, пока не выполнится одно из следующих условий:
	// 1) d достигнет n-1 
	// 2) (x^2) % n не 1 
	// 3) (x^2) % n не n-1 
	while (d != n - 1)
	{
		x = (x * x) % n;
		d *= 2;
		if (x == 1)
			return false;
		if (x == n - 1)
			return true;
	}

	// вернуть "составное"
	return false;
}

/* 
 Функция проверки простоты числа n за k раундов
* Возвращает false, если n составное; возвращает true, если n возможно простое
* Переменная k - входной параметр, который влияет на точность проверки. 
*/
bool isPrime(int n, int k)
{
	if (n <= 1 || n == 4)  
		return false;
	if (n <= 3) 
		return true;

	// Найти r такое, что n = 2^d * r + 1 для некоторого r >= 1 
	int d = n - 1;
	while (d % 2 == 0)
		d /= 2;

	// Проверить число вероятностным тестом простоты k раз
	for (int i = 0; i < k; i++)
		if (!millerTest(n, d))
			return false;

	return true;
}

/*
 Функция генерации простого числа
*/
int generate_prime()
{
	int generated = rand() % LIMIT; // сгенерировать число
	while (!isPrime(generated, log(generated))) // генерировать еще, пока оно не окажется простым
		generated = rand() % LIMIT;
	return generated; // вернуть простое число
}

/*
Алгоритм Евклида (https://ru.wikipedia.org/wiki/Алгоритм_Евклида)
* Алгоритм для нахождения наибольшего общего делителя двух целых чисел.
* Используется для проверки чисел на взаимную простоту
*/
int gcd(int a, int b)
{
	while (b)
	{
		int r = a % b;
		a = b;
		b = r;
	}
	return a;
}

/*
 Функция генерации числа, взаимно простого с числом n
*/
int generate_coprime(int n)
{
	int generated = rand() % LIMIT;  // сгенерировать число
	while (gcd(n, generated) != 1)  // генерировать еще, пока оно не окажется взаимно простым с n
		generated = rand() % LIMIT;
	return generated; // вернуть взаимно простое с n число
}

/*
 Расширенный алгоритм Евклида (см. там же, где и обычный алгоритм)
* или (http://e-maxx.ru/algo/export_extended_euclid_algorithm)
* Возвращает пару y, x: a * x + b * y = gcd(a, b)
*/
pair<int, int> euclid_extended(int a, int b) 
{
	if (!b)				// если b = 0, то x = 0, y = 1 - база рекурсии
		return { 1, 0 };
	// иначе рекурсивный пересчет коэффициентов
	auto result = euclid_extended(b, a % b);
	return { result.second, result.first - (a / b) * result.second };
}

/*
 Модулярная инверсия.
* Ищет такое x, что a * x ≡ 1 (mod m)
* 1) Чтобы найти x, необходимо положить b = m в формуле расширенного алгоритма Евклида ax + by = gcd(a, b). 
* Поскольку известно, что a и m взаимно просты, можно положить значение gcd как 1: ax + my = 1 
* 2) Если взять по модулю m с обеих сторон, то получится ax + my ≡ 1 (mod m)
* 3) Можно удалить второй член слева, так как my (mod m) всегда будет равно нулю для целого числа y: ax ≡ 1 (mod m)
* Таким образом, используя расширенный алгоритм Евклида, можно найти x
*/
int modular_inverse(int a, int m)
{
	int x = euclid_extended(a, m).first;
	// m добавляется в случае отрицательного x:
	while (x < 0)
		x += m; 
	return x;
}

typedef pair<int, int> PublicKey; // пара (e, n) - открытый ключ
typedef pair<int, int> PrivateKey; // пара (d, n) - закрытый ключ

/* 
Структура Ключи – открытый и закрытый
*/
struct Keys
{
	PublicKey public_key;
	PrivateKey private_key;
};

/*
 Функция генерации ключей
*/
Keys generate_keys()
{
	Keys result; // возвращаемое значение

	int p, q; // два случайных простых числа

	// Сгенерировать их:
	p = generate_prime();
	q = generate_prime();

	// Вычислить модуль - произведение сгенерированных простых чисел:
	int n = p * q;

	// Найти phi - значение функции Эйлера от числа n:
	int phi = (p - 1) * (q - 1);

	// Выбрать число e, взаимно простое с числом phi:
	int e = generate_coprime(phi);

	// Выбрать пару (e, n) в качестве открытого ключа:
	result.public_key = make_pair(e, n);

	// Найти d - мультипликативно обратное к числу e мо модулю phi: 
	int d = modular_inverse(e, phi);

	// Выбрать пару (d, n) в качестве закрытого ключа:
	result.private_key = make_pair(d, n);

	return result; // вернуть результат
}

/*
 Функция шифрования
*/
int encrypt(PublicKey key, int value)
{
	return modulo(value, key.first, key.second);
}

/*
Функция дешифрования
*/
int decrypt(PrivateKey key, int value)
{
	return modulo(value, key.first, key.second);
}

int main()
{
	srand(time(NULL));  // инициализация генератора случайных чисел

	// Установка ввода-вывода кириллицы в консоли:
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	Keys keys = generate_keys(); // генерация ключей keys

	// Вывод сгенерированных ключей:
	cout << "Сгенерированный открытый ключ: (" << keys.public_key.first << ", " << keys.public_key.second << ")" << endl;;
	cout << "Сгенерированный закрытый ключ: (" << keys.private_key.first << ", " << keys.private_key.second << ")" << endl;

	// Чтение строки с пробелами и нахождение ее длины:
	string s;
	cout << endl << "Введите строку: ";
	getline(cin, s);
	int len = s.length();

	// Вывод исходной строки и кодов ее символов: 
	cout << "Исходная строка: " << s << endl;
	cout << endl << "Коды символов исходной строки: " << endl;
	for (int i = 0; i < len; i++)
		cout << (int)s[i] << " ";
	cout << endl;

	// Объявление двух целочисленных массивов для хранения кодов шифртекста и расшифрованного текста и заполнение их нулями:
	int enc[100] = { 0 };
	int dec[100] = { 0 };

	// Посимвольное шифрование строки:
	for (int i = 0; i < len; i++)
		enc[i] = encrypt(keys.public_key, s[i]);

	// Вывод кодов символов шифртекста:
	cout << endl << "Коды символов шифртекста: " << endl;
	for (int i = 0; i < len; i++)
		cout << enc[i] << " ";
	cout << endl;

	// Посимвольное дешифрование строки:
	for (int i = 0; i < len; i++)
		dec[i] = decrypt(keys.private_key, enc[i]);

	// Вывод кодов символов дешифрованного текста:
	cout << endl << "Дешифрованные коды: " << endl;
	for (int i = 0; i < len; i++)
		cout << dec[i] << " ";
	cout << endl;

	// Вывод символов дешифрованной строки:
	cout << endl << "Дешифрованная строка: " << endl;
	for (int i = 0; i < len; i++)
		cout << (char)dec[i];
	cout << endl << endl;
	system("pause");
	return 0;
}
