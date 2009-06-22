"""Compound tests

.. author:: Thomas Cokelaer, Thomas.Cokelaer@inria.fr

"""
__revision__ = "$Id$"

from openalea.stat_tool.compound import Compound
from openalea.stat_tool.data_transform import ExtractDistribution
from openalea.stat_tool.distribution import Binomial, NegativeBinomial

from openalea.stat_tool.estimate import Estimate
from openalea.stat_tool.simulate import Simulate
from openalea.stat_tool.data_transform import ExtractHistogram, ExtractData, \
    Shift
from openalea.stat_tool.histogram import Histogram
from openalea.stat_tool.distribution import ToHistogram

from tools import interface

class Test(interface):
    """a simple unittest class"""
    def __init__(self):
        interface.__init__(self, 
            self.build_data(), 
            "data/compound1.cd", 
            Compound)
        
    def build_data(self):
        d1 = Binomial(2, 5, 0.5)
        d2 = NegativeBinomial(0, 2, 0.5)
        data = Compound(d1, d2) 
        return data
        
    def test_empty(self):
        self.empty()

    def test_constructor_from_file(self):
        self.constructor_from_file()

    def test_constructor_from_file_failure(self):
        self.constructor_from_file_failure()

    def test_constructor_from_compound(self):
        compound1 = self.data
        compound2 = Compound(compound1)
        assert str(compound1) == str(compound2)
        
    def test_constructor_from_dists_and_threshold(self):
        # by default, the COMPOUND_THRESHOLD is harcoded to be 0.99999
        compound1 = self.data
        compound2 = Compound(Binomial(2, 5, 0.5),
                             NegativeBinomial(0, 2, 0.5),
                             0.99999)
        assert str(compound1) == str(compound2)

        
        
    def test_print(self):
        self.print_data()
        
    def test_display(self):
        self.display()
        self.display_versus_ascii_write()
        self.display_versus_str()
        
    def test_len(self):
        """not implemented; irrelevant?"""
        pass
    
    def test_plot(self):        
        self.plot()

    def test_save(self):
        self.save()
                
    def test_plot_write(self):
        self.plot_write()

    def test_file_ascii_write(self):
        self.file_ascii_write()
      
    def test_spreadsheet_write(self):
        self.spreadsheet_write()
    
    def test_simulate(self):
        self.simulate()

    def test_extract(self):
        """run and test the extract methods"""
        m = self.data
        assert m.extract_compound() == ExtractDistribution(m, "Compound")
        assert m.extract_sum() == Binomial(2, 5, 0.5)
        assert m.extract_sum() == ExtractDistribution(m, "Sum")
        assert m.extract_elementary() == NegativeBinomial(0, 2, 0.5)
        assert m.extract_elementary() == ExtractDistribution(m, "Elementary")

    def test_extract_data(self):
        """todo : check if this test makes sense"""
        
        s = self.simulate()
        e = s.estimate_compound(Binomial(2, 5, 0.5))
        d = e.extract_data()
        assert d
        _eprime = Estimate(s, "COMPOUND", Binomial(0, 10, 0.5), 
                           NegativeBinomial(0, 2, 0.5))

    
def test1():
    """Various tests on compound data
    
    Those tests are done elsewhere:
     * Estimate in test_estimate
     * ExtractHistogram in test_data_transform
     ...
    """
    cdist1 = Compound("data/compound1.cd")
    chisto1 = Simulate(cdist1, 200)
    _histo30 = ExtractHistogram(chisto1, "Sum")

    cdist2 = Estimate(chisto1, "COMPOUND",
                      ExtractDistribution(cdist1, "Elementary"),
                      ExtractDistribution(cdist1, "Sum"),
                      MinInfBound=0)
    
    _histo31 = ExtractHistogram(ExtractData(cdist2), "Sum")
    _histo32 = ToHistogram(ExtractDistribution(cdist2, "Sum"))
    
    peup1 = Histogram("data/peup1.his")
    mixt4 = Estimate(peup1, "MIXTURE", "B", "NB")
    histo33 = ToHistogram(ExtractDistribution(mixt4, "Component", 2))
    _histo34 = Shift(histo33, -11)

    #_cdist3 = Estimate(histo34, "COMPOUND",
#                      Distribution("B", 0, 1, 0.7),
#                      ExtractDistribution(histo34, "Sum"))
    #_cdist4 = Estimate(histo34, "COMPOUND",
    #                  Distribution("B", 0, 1, 0.7),
    #                  ExtractDistribution(histo34, "Sum"), MinInfBound=0)

