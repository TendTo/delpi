import pydelpi as pdi


class TestFormula:
    def setup_method(self):
        self.x_ = pdi.Variable("x")
        self.y_ = pdi.Variable("y")
        self.z_ = pdi.Variable("z")
        self.e1_ = 2 * self.x_ + 3 * self.y_ + 4 * self.z_
        self.e2_ = self.x_ - self.y_ + 2 * self.z_

    def test_constructor(self):
        f = pdi.Formula(self.e1_, pdi.FormulaKind.EQ, 2)
        assert f.expression.equal_to(self.e1_)
        assert f.kind == pdi.FormulaKind.EQ
        assert f.rhs == 2

    def test_equality(self):
        f1 = pdi.Formula(self.e1_, pdi.FormulaKind.EQ, 2)
        f2 = pdi.Formula(self.e1_, pdi.FormulaKind.EQ, 2)
        f3 = pdi.Formula(self.e2_, pdi.FormulaKind.NEQ, 3)

        assert f1.equal_to(f2)
        assert not f1.equal_to(f3)

    def test_less(self):
        f1 = pdi.Formula(self.e1_, pdi.FormulaKind.EQ, 2)
        f2 = pdi.Formula(self.e2_, pdi.FormulaKind.NEQ, 3)

        assert f1.less(f2) or f2.less(f1)

    def test_hash(self):
        f1 = pdi.Formula(self.e1_, pdi.FormulaKind.EQ, 2)
        f2 = pdi.Formula(self.e1_, pdi.FormulaKind.EQ, 2)
        f3 = pdi.Formula(self.e2_, pdi.FormulaKind.NEQ, 3)

        assert hash(f1) == hash(f2)
        assert hash(f1) != hash(f3)

    def test_operator_equals(self):
        f = self.x_ == self.y_
        assert f.kind == pdi.FormulaKind.EQ

    def test_operator_not_equals(self):
        f = self.x_ != self.y_
        assert f.kind == pdi.FormulaKind.NEQ

    def test_operator_less_than(self):
        f = self.x_ < self.y_
        assert f.kind == pdi.FormulaKind.LT

    def test_operator_less_than_or_equal(self):
        f = self.x_ <= self.y_
        assert f.kind == pdi.FormulaKind.LEQ

    def test_operator_greater_than(self):
        f = self.x_ > self.y_
        assert f.kind == pdi.FormulaKind.GT

    def test_operator_greater_than_or_equal(self):
        f = self.x_ >= self.y_
        assert f.kind == pdi.FormulaKind.GEQ

    def test_str(self):
        f = pdi.Formula(self.e1_, pdi.FormulaKind.EQ, 2)
        assert str(f) == "((2 * x + 3 * y + 4 * z) = 2)"
