"""Distribution tests"""
__revision__ = "$Id: test_distribution.py 6219 2009-04-08 14:11:08Z cokelaer $"


from openalea.stat_tool import _stat_tool
from openalea.sequence_analysis import _sequence_analysis
from openalea.sequence_analysis.sequences import Sequences

from openalea.stat_tool.data_transform import * 
from openalea.stat_tool.cluster import Cluster 

from tools import interface

class Test(interface):
    """a simple unittest class

    Integration Test 
    ================
    
    * 'ok' means works and testedPerform test on 
        
    ========================    ==================================
    ** from the interface**
    ascii_write                 ok
    display                     ok
    extract_data                nothing to be done
    file_ascii_write            ok
    plot                        ok                       
    save                        ok
    plot_print                  ok
    simulate                    ok
    plot_write                  ok
    spreadsheet_write           ok

    **others**
    old_plot                    ok   
    str                         ok
    len                         not relevant
    ========================    ==================================
 
    """
    def __init__(self):
        interface.__init__(self,
                           self.build_data(),
                           "sequences1.seq",
                           Sequences)
        self.seqn = self.build_seqn()
        self.seq1 = self.build_seq1()
        
    def build_data(self):
        """todo: check identifier output. should be a list """
        # build a list of 2 sequences with a variable that should be identical
        # to sequences1.seq
        data = Sequences([
                    [1, 0, 0, 0, 1, 1, 2, 0, 2, 2, 2, 1, 1, 0, 1, 0, \
                     1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 2, 2, 2, 1],
                    [0, 0, 0, 1, 1, 0, 2, 0, 2, 2 ,2 ,1 ,1 ,1 ,1 ,0 ,1 \
                     ,0 ,0 ,0 ,0 ,0]])
        assert data

        assert data.get_nb_sequence() == 2
        assert data.get_nb_variable() == 1
        assert [0, 1] == data.get_identifiers()
        
        return data        
    
    def build_seqn(self):
        s = Sequences([[[1,1,1],[12,12,12]],[[2,2,2],[22,22,22]]])
        return s
    
    def build_seq1(self):
        s = Sequences([[1,1,1],[2,2,2]])
        return s
    
    def test_empty(self):
        self.empty()

    def test_constructor_from_file(self):
        self.constructor_from_file()

    def test_constructor_from_file_failure(self):
        self.constructor_from_file_failure()

    def test_print(self):
        self.print_data()
        
    def test_display(self):
        self.display()
        self.display_versus_ascii_write()
        self.display_versus_str()
        
    def test_len(self):
        seq = self.data
        assert len(seq) == 2
        assert len(seq) == seq.get_nb_sequence()

    def test_plot(self):        
        self.plot()
    
    def test_save(self):
        self.save(skip_reading=True)
                    
    def test_plot_write(self):
        self.plot_write()
        
    def test_file_ascii_write(self):
        self.file_ascii_write()
        
    def test_spreadsheet_write(self):
        self.spreadsheet_write()
        
    def test_simulate(self):
        #self.simulate()
        pass
        
        
    def test_extract(self):
        #todo
        pass

    def test_extract_data(self):
        pass 

    def test_sequences_one_variable(self):
        # two sequences with 2 variables (int)
        s = Sequences([[1,1,1],[2,2,2]])
        assert s
        # two sequences with 2 variables (real)
        s = Sequences([[1.,1.,1.],[2.,2.,2.]])
        assert s
        # two sequences with 2 variables mix of (real and int)
        # works because the first one is float so all others are assume to be float as well
        s = Sequences([[1.,1.,1.],[2.,2.,2]])
        assert s
        # here it fails because the first number is int but others may be float
        try:
            s = Sequences([[1,1.,1.],[2.,2.,2]])
            assert False
        except:
            assert True
            
    def test_sequences_two_variables(self):
        # two sequences with 2 variables (int)
        s = Sequences([[[1,1,1],[12,12,12]],[[2,2,2],[22,22,22]]])
        assert s
        s = Sequences([[[1.,1.,1.],[1.,1.,1.]],[[2.,2.,2.],[2.,2.,2.]]])
        assert s
        
    def test_container(self):
        # first index is the sequence and second index is the variable
        s = self.seqn
        assert s[0,0] == [1,1,1]
        assert s[0,1] == [12,12,12]
        assert s[1,0] == [2,2,2]
        assert s[1,1] == [22,22,22]
        assert s[1,1][1] == 22
        assert len(s) == 2
        
        s = self.seq1
        assert s[0,0] == [1,1,1]
        assert s[1,0] == [2,2,2]
        assert s[0,0][0] == 1
        assert len(s) == 2
        

       
    def test_value_select(self):
        "test_value_select implemented but need to be checked"
        seqn = self.seqn
        a = seqn.value_select(1, 1, 2,True)
        assert a
        
    
    def test_select_variable(self):
        "test_select_variable implemented but need to be checked (index issue)"
        # !!!!!!!! NEED to CHECK THE INDEX 0, 1 , ... or 1,2,....
        # what about identifiers ? 
        # Variable seems to start at 1 not 0
        s = self.seqn
        #select variable 1
        select = s.select_variable([1], keep=True)
        assert select[0,0] == [1, 1, 1]
        assert select[1,0] == [2, 2, 2]

    
    def test_select_individual(self):
        #select one or several sequences
        s = self.seqn
        
        # select all
        select = s.select_individual([0,1], keep=True)
        assert s.display() == select.display()
        
        #select first sequence only 
        select = s.select_individual([0], keep=True)
        assert select[0,0] == [1, 1, 1]
        assert select[0,1] == [12, 12, 12]
        try:
            select[1,0]
            assert False
        except:
            assert True
        
        #select second sequence only 
        select = s.select_individual([1], keep=True)
        assert select[0,0] == [2, 2, 2]
        assert select[0,1] == [22, 22, 22]
        try:
            select[1,0]
            assert False
        except:
            assert True

    def test_shift_seqn(self):
        s = self.seqn
        shifted = s.shift(1,2)
        assert shifted[0,0] == [3,3,3]
        assert shifted[0,1] == [12,12,12]
        assert shifted[1,0] == [4,4,4]
        assert shifted[1,1] == [22,22,22]

    def test_shit_seq1(self):
        s = self.seq1
        shifted = s.shift(1,2)
        assert shifted[0,0] == [3,3,3]
        assert shifted[1,0] == [4,4,4]
        
    def test_merge (self):
        s1 = self.seqn
        s2 = self.seqn
        s3 = self.seq1
        
        sall = s1.merge([s2])
        
        assert sall.get_nb_sequence() == 4
        assert sall.get_nb_variable() == 2
            
    def test_merge_and_Merge(self):
        s1 = self.seqn
        s2 = self.seqn
        
        a = s1.merge([s2])
        b = s2.merge([s1])
        v = Merge(s1,s2)

        assert str(a) == str(b)
        assert str(a) == str(v)

    def test_imcompatible_merge(self):
        s1 = self.seqn
        s3 = self.seq1
        try:
            s1.merge(s3)
            assert False
        except:
            assert True
        
    def test_merge_variable(self):
        s1 = self.seqn
        s2 = self.seqn
        s3 = self.seq1
        
        sall = s1.merge_variable([s2],1) # why 1 ? same result with 2 !
        assert sall.get_nb_sequence() == 2
        assert sall.get_nb_variable() == 4
        
        sall =  s1.merge_variable([s3],1)
        assert sall.get_nb_sequence() == 2
        assert sall.get_nb_variable() == 3 

    def test_merge_variable_and_MergeVariable(self):
        s1 = self.seqn
        s2 = self.seqn
        s3 = self.seq1

        a = s1.merge_variable([s2],1)
        b = s2.merge_variable([s1],1)
        v = MergeVariable(s1,s2)

        assert str(a) == str(b)
        assert str(a) == str(v)

    def test_cluster_limit(self):
        "implemented but need a test"
        
    def test_cluster_step(self):
        seq1 = Sequences([[1, 2, 3], [1, 3, 1], [4, 5, 6]])
        assert str(Cluster(seq1, "Step", 2)) == str(seq1.cluster_step(1, 2))
        seqn = Sequences([[[1, 2, 3], [1, 3, 1]], [[4, 5, 6], [7,8,9]]])
        assert str(Cluster(seqn, "Step", 1, 2)) == str(seqn.cluster_step(1, 2))
       
    def test_cluster_limit(self):
        seq1 = Sequences([[1, 2, 3], [1, 3, 1], [4, 5, 6]])
        assert str(Cluster(seq1, "Limit", [2, 4, 6])) == \
            str(seq1.cluster_limit(1, [2, 4 ,6]))
        seqn = Sequences([[[1, 2, 3], [1, 3, 1]], [[4, 5, 6], [7,8,9]]])
        assert str(Cluster(seqn, "Limit", 1, [2, 4, 6])) == \
            str(seqn.cluster_limit(1, [2, 4 ,6]))
    
        
        
    def _others(self):
        #cluster, cumulate, difference, indexextract, lengthselect, merge
        #mergevariable, moving average, reccurrenceTimeSequences, removerun
        #reverse, segmentatoinExtravt, selectindividual, selectvariable, shift, 
        #transcode, valueselect, variablescaling, transormposition
        pass