import unittest
from Cube import cubic
mass0_1 = [1,4,-9,-36]
mass0_2 = [1,2,-1,-2]

mass1_1 = [3,-4,-3]
mass1_2 = [1,-2,-1]

mass2_1 = [0,1,35,3]
mass2_2 = [5,0,2,1]
mass2_3 = [5,2,0,1]
mass2_4 = [5,2,2,0]

mass3_1 = ["test",4,5,2]
mass3_2 = [2j,4,5,2]
mass3_3 = [True,4,5,2]
mass3_4 = [[5,2],4,5,2]
class TestCube(unittest.TestCase):
    
    def test_cubic(self):
        self.assertEqual(cubic(mass0_1),mass1_1) #Сравнение решения при обычных цифрах
        self.assertEqual(cubic(mass0_2),mass1_2)
        
    def test_cubic2(self):
        self.assertRaises(ValueError, cubic, mass2_1)
        self.assertRaises(ValueError, cubic, mass2_2)
        self.assertRaises(ValueError, cubic, mass2_3)
        self.assertRaises(ValueError, cubic, mass2_4)
    
    def test_cubic3(self):
        self.assertRaises(TypeError, cubic, mass3_1)
        self.assertRaises(TypeError, cubic, mass3_2)
        self.assertRaises(TypeError, cubic, mass3_3)
        self.assertRaises(TypeError, cubic, mass3_4)

if __name__ == '__main__':
    unittest.main()