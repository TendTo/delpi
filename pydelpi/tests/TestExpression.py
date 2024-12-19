import pytest

import pydelpi as pdi


class TestExpression:
    def setup_method(self):
        self.x_ = pdi.Variable("x")
        self.y_ = pdi.Variable("y")
        self.z_ = pdi.Variable("z")
        self.e1_ = pdi.Expression()
        self.e2_ = pdi.Expression()

    def test_default_constructor(self):
        e = pdi.Expression()
        assert not e.addends
        assert hash(e) == 0
        assert e.equal_to(e)
        assert not e.less(e)
        assert not e.variables
        assert e.evaluate({}) == 0
        assert not e.substitute({}).addends

    def test_variable_constructor(self):
        e = pdi.Expression(self.x_)
        assert e.addends[self.x_] == 1
        assert hash(e) != 0
        assert e.equal_to(e)
        assert not e.less(e)
        assert e.variables[0].equal_to(self.x_)
        assert e.evaluate({self.x_: 1}) == 1
        assert e.evaluate({self.x_: 2}) == 2

    def test_linear_monomial_constructor(self):
        e = pdi.Expression({self.x_: 2})
        assert e.addends[self.x_] == 2
        assert hash(e) != 0
        assert e.equal_to(e)
        assert not e.less(e)
        assert e.variables[0].equal_to(self.x_)
        assert e.evaluate({self.x_: 1}) == 2
        assert e.evaluate({self.x_: 2}) == 4

    def test_addends_constructor(self):
        e = pdi.Expression({self.x_: 1, self.y_: 2, self.z_: 6})
        assert e.addends[self.x_] == 1
        assert e.addends[self.y_] == 2
        assert e.addends[self.z_] == 6
        assert hash(e) != 0
        assert e.equal_to(e)
        assert not e.less(e)
        assert e.variables[0].equal_to(self.x_)
        assert e.variables[1].equal_to(self.y_)
        assert e.variables[2].equal_to(self.z_)
        assert e.evaluate({self.x_: 1, self.y_: 2, self.z_: 3}) == 1 + 4 + 18

    def test_copy_constructor(self):
        e = pdi.Expression(self.e1_)
        assert e.use_count == 2
        assert e.use_count == 2

        e.add(self.x_, 1)
        assert e.use_count == 1
        assert e.use_count == 1
        assert e.variables[0].equal_to(self.x_)

    def test_copy_constructor_reference_count(self):
        e = pdi.Expression({self.x_: 2})
        assert e.use_count == 1
        e1 = pdi.Expression(e)
        assert e.use_count == 2
        e2 = pdi.Expression(e)
        assert e.use_count == 3

    def test_copy_reference_count(self):
        e = pdi.Expression({self.x_: 2})
        assert e.use_count == 1
        e1 = e.copy()
        assert e.use_count == 2
        e2 = e1.copy()
        assert e.use_count == 3

    def test_reference_count(self):
        e = pdi.Expression({self.x_: 2})
        assert e.use_count == 1
        e1 = e
        assert e1.use_count == 1
        e2 = e1
        assert e2.use_count == 1

        e.add(self.y_, 1)
        assert e.addends == {self.x_: 2, self.y_: 1}
        assert e1.addends == {self.x_: 2, self.y_: 1}
        assert e2.addends == {self.x_: 2, self.y_: 1}

    def test_hash(self):
        assert hash(self.e1_) == hash(self.e2_)
        c1 = self.e1_.copy()
        assert hash(c1) == hash(self.e1_)
        self.e1_.add(self.x_, 4)
        assert hash(c1) != hash(self.e1_)
        c2 = self.e1_.copy()
        assert hash(c2) == hash(self.e1_)
        self.e1_ *= 2
        assert hash(c2) != hash(self.e1_)
        c3 = self.e1_.copy()
        assert hash(c3) == hash(self.e1_)
        self.e1_ /= 2
        assert hash(c3) != hash(self.e1_)

        assert hash(c2) == hash(self.e1_)
        self.e1_.add(self.x_, -4)
        assert hash(self.e1_) == hash(self.e2_)

    def test_add(self):
        self.e1_.add(self.x_, 1)
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.addends[self.x_] == 1

        self.e1_.add(self.x_, 4)
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.addends[self.x_] == 5

        self.e1_.add(self.x_, -6)
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.addends[self.x_] == -1

        self.e1_.add(self.y_, -7)
        assert self.e1_.variables[1].equal_to(self.y_)
        assert self.e1_.addends[self.y_] == -7

        self.e1_.add(self.y_, 7)
        assert self.e1_.variables[0].equal_to(self.x_)

    def test_multiply(self):
        self.e1_.add(self.x_, 7)
        self.e1_.add(self.y_, 12)
        self.e1_ *= 2
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.variables[1].equal_to(self.y_)
        assert self.e1_.addends[self.x_] == 14
        assert self.e1_.addends[self.y_] == 24

        self.e1_ *= 1
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.variables[1].equal_to(self.y_)
        assert self.e1_.addends[self.x_] == 14
        assert self.e1_.addends[self.y_] == 24

        self.e1_ *= -1
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.variables[1].equal_to(self.y_)
        assert self.e1_.addends[self.x_] == -14
        assert self.e1_.addends[self.y_] == -24

        self.e1_.add(self.x_, 1)
        self.e1_.add(self.y_, 1)
        self.e1_.add(self.z_, 1)
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.variables[1].equal_to(self.y_)
        assert self.e1_.variables[2].equal_to(self.z_)
        assert self.e1_.addends[self.x_] == -13
        assert self.e1_.addends[self.y_] == -23
        assert self.e1_.addends[self.z_] == 1

        self.e1_ *= 0
        assert not self.e1_.variables
        assert not self.e1_.addends

    def test_divide(self):
        self.e1_.add(self.x_, 18)
        self.e1_.add(self.y_, 12)
        self.e1_ /= 2
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.variables[1].equal_to(self.y_)
        assert self.e1_.addends[self.x_] == 9
        assert self.e1_.addends[self.y_] == 6

        self.e1_ /= 1
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.variables[1].equal_to(self.y_)
        assert self.e1_.addends[self.x_] == 9
        assert self.e1_.addends[self.y_] == 6

        self.e1_ /= -1
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.variables[1].equal_to(self.y_)
        assert self.e1_.addends[self.x_] == -9
        assert self.e1_.addends[self.y_] == -6

        self.e1_.add(self.x_, 1)
        self.e1_.add(self.y_, 1)
        self.e1_.add(self.z_, 1)
        assert self.e1_.variables[0].equal_to(self.x_)
        assert self.e1_.variables[1].equal_to(self.y_)
        assert self.e1_.variables[2].equal_to(self.z_)
        assert self.e1_.addends[self.x_] == -8
        assert self.e1_.addends[self.y_] == -5
        assert self.e1_.addends[self.z_] == 1

        with pytest.raises(RuntimeError):
            assert self.e1_ / 0

    def test_negative_variable(self):
        e = pdi.Expression(-self.x_)
        assert e.addends[self.x_] == -1

    def test_complex_expressions(self):
        e1 = pdi.Expression(self.x_ + 2 * self.y_ + 3 * self.z_)
        assert e1.variables[0].equal_to(self.x_)
        assert e1.variables[1].equal_to(self.y_)
        assert e1.variables[2].equal_to(self.z_)
        assert e1.evaluate({self.x_: 1, self.y_: 2, self.z_: 3}) == 14

        e2 = pdi.Expression(
            self.x_
            + 2 * self.y_
            + 3 * self.z_
            + self.z_
            + 4 * self.y_ * 5
            + 6 * self.x_ * 7
            - 8 * self.x_
            - self.z_ * 4
            + self.x_ / 2
        )
        assert len(e2.variables) == 2
        assert e2.variables[0].equal_to(self.x_)
        assert e2.variables[1].equal_to(self.y_)
        assert e2.evaluate({self.x_: 1, self.y_: 2}) == 79 + 1 / 2

    def test_sum_expression(self):
        e1 = pdi.Expression(self.x_ + 2 * self.y_ + 3 * self.z_ + self.z_ + 4 * self.y_ * 5)
        e2 = pdi.Expression(6 * self.x_ * 7 - 8 * self.x_ - self.z_ * 4 + self.x_ / 2)
        assert (e1 + e2).variables[0].equal_to(self.x_)
        assert (e1 + e2).variables[1].equal_to(self.y_)
        assert (e1 + e2).evaluate({self.x_: 1, self.y_: 2}) == 79 + 1 / 2

    def test_subtract_expression(self):
        e1 = pdi.Expression(self.x_ + 2 * self.y_ + 3 * self.z_ + self.z_ + 4 * self.y_ * 5 - 8 * self.z_)
        e2 = pdi.Expression(6 * self.x_ * 7 - 8 * self.x_ - self.z_ * 4 + self.x_ / 2)
        assert (e1 - e2).variables[0].equal_to(self.x_)
        assert (e1 - e2).variables[1].equal_to(self.y_)
        assert (e1 - e2).evaluate({self.x_: 1, self.y_: 2}) == -67 / 2 + 44
