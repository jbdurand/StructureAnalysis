""" Cluster tests

.. author:: Thomas Cokelaer, Thomas.Cokelaer@inria.fr

.. todo:: check the AddVariable option (sequences) and sequences cases
"""
__revision__ = "$Id:  $"


from openalea.sequence_analysis.sequences import Sequences
#from openalea.sequence_analysis.semi_markov import SemiMarkov
from openalea.stat_tool.cluster import Cluster
from openalea.stat_tool.histogram import Histogram
from openalea.stat_tool.convolution import Convolution
from openalea.stat_tool.compound import Compound
from openalea.stat_tool.vectors import Vectors



class _Cluster():
    """Test class to test cluster function and classes
    
    create_data, cluster_step and cluster_limit funciton will be required
    """
    
    def __init__(self):
        self.data = None    
    
    def create_data(self):
        raise NotImplemented
        
    def test_cluster_step(self):
        raise NotImplemented
    
    def test_cluster_limit(self):
        raise NotImplemented

class _HistoCase(_Cluster):
    """
    inherits from _cluster and implements the cluster_limit and cluster_step 
    functions. 
    
    In addition, classes that inherits from _HistoCase must implement 
    cluster_information 
    """
    def __init__(self):
        _Cluster.__init__(self)
        self.data = None
        
    def test_cluster_step(self):
        cluster1 = Cluster(self.data, "Step", 2)
        cluster2 = self.data.cluster_step(2)
        assert str(cluster1) == str(cluster2)
        
    def test_cluster_limit(self):
        cluster1 = Cluster(self.data, "Limit", [2, 4, 6, 8, 10])
        cluster2 = self.data.cluster_limit([2, 4, 6, 8, 10])
        assert str(cluster1) == str(cluster2)

    def test_cluster_information(self):
        cluster1 = Cluster(self.data, "Information", 0.8)
        cluster2 = self.data.cluster_information(0.8)
        assert str(cluster1) == str(cluster2)

    
class TestHistogram(_HistoCase):
    
    def __init__(self):
        _HistoCase.__init__(self)
        self.data = self.create_data()
    
    def create_data(self):
        return Histogram('data/fagus1.his')

    
class TestConvolution( _HistoCase):
    
    def __init__(self):
        _HistoCase.__init__(self)
        self.data = self.create_data()
    
    def create_data(self):
        conv = Convolution('data/convolution1.conv')
        return conv.simulate(1000)

    
class TestCompound(_HistoCase):
    
    def __init__(self):
        _HistoCase.__init__(self)
        self.data = self.create_data()
    
    def create_data(self):
        comp = Compound('data/compound1.cd')
        return comp.simulate(1000)


class TestVectorsn(_Cluster):

    def __init__(self):
        _Cluster.__init__(self)
        self.data = self.create_data()
        
    def create_data(self):
        v = Vectors([[1, 2, 3], [1, 3, 1], [4, 5, 6]])
        return v

    def test_cluster_step(self):
        data = self.data
        cluster1 = data.cluster_step(1, 2)
        cluster2 = Cluster(data, "Step", 1, 2)
        assert str(cluster1) == str(cluster2)
        
    def test_cluster_limit(self):
        data = self.data
        cluster1 = data.cluster_limit(1, [2, 4, 6])
        cluster2 = Cluster(data, "Limit", 1, [2, 4, 6])
        assert str(cluster1) == str(cluster2)
        
class TestVectors1(_Cluster):

    def __init__(self):
        _Cluster.__init__(self)
        self.data = self.create_data()
        
    def create_data(self):
        v = Vectors([[1], [1], [4]])
        return v

    def test_cluster_step(self):
        data = self.data
        cluster1 = data.cluster_step(1, 2)
        cluster2 = Cluster(data, "Step", 2)
        assert str(cluster1) == str(cluster2)
        
    def test_cluster_limit(self):
        data = self.data
        cluster1 = data.cluster_limit(1, [2, 4, 6])
        cluster2 = Cluster(data, "Limit", [2, 4, 6])
        assert str(cluster1) == str(cluster2)


class TestSequences1(_Cluster):

    def __init__(self):
        _Cluster.__init__(self)
        self.data = self.create_data()
        
    def create_data(self):
        data = Sequences('data/sequences1.seq')
        return data

    def test_cluster_step(self):
        data = self.data
        mode = False
        cluster1 = data.cluster_step(1, 2, mode)
        cluster2 = Cluster(data, "Step", 2)
        assert str(cluster1) == str(cluster2)
        
    def test_cluster_limit(self):
        data = self.data
        cluster1 = data.cluster_limit(1,[2], False)
        cluster2 = Cluster(data,"Limit",[2])
        assert str(cluster1) == str(cluster2)
        
        
class TestSequencesn(_Cluster):

    def __init__(self):
        _Cluster.__init__(self)
        self.data = self.create_data()
        
    def create_data(self):
        data = Sequences('data/sequences2.seq')
        return data

    def test_cluster_step(self):
        data = self.data
        mode = False
        cluster1 = data.cluster_step(1, 2, mode)
        cluster2 = Cluster(data, "Step", 1, 2)
        assert str(cluster1) == str(cluster2)
        
    def test_cluster_limit(self):
        data = self.data
        cluster1 = data.cluster_limit(1, [2 ], False)
        cluster2 = Cluster(data, "Limit", 1, [2])
        assert str(cluster1) == str(cluster2)
        
