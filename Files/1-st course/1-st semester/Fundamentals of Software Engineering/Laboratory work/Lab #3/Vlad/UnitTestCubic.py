import unittest
from Equations import cubic
from Equations import quadratic

massive_coef_cubic = [1, 2, -1, -2] #коэффициенты кубического уравнения
massive_ans_cubic = [1, -2, -1] #ожидаемые ответы для кубического уравнения

massive_coef_quadratic = [1, 10, -39] #коэффициенты квадратного уравнения
massive_ans_quadratic = [3, -13] #ожидаемые ответы для квадратного уравнения


class TestEquations (unittest.TestCase):
    def test_equations1(self): #тест на получение определённых корней при определённых коэффиицентах
        self.assertEqual(cubic(massive_coef_cubic), massive_ans_cubic)
        self.assertEqual(quadratic(massive_coef_quadratic), massive_ans_quadratic)
        
    def test_equations2(self): #тест на получение корней, а не None значений
        self.assertIsNotNone(cubic(massive_coef_cubic))
        self.assertIsNotNone(quadratic(massive_coef_quadratic))
        
    def test_equations3(self): #тест проверяет, чтобы функция не выдавали корень = 0 при определённых коэффициентах
        self.assertNotIn(0, cubic(massive_coef_cubic))
        self.assertNotIn(0, quadratic(massive_coef_quadratic))
        
if __name__ == '__main__':
    unittest.main()