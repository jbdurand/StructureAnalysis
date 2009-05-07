/*------------------------------------------------------------------------------
 *
 *        VPlants.Stat_Tool : VPlants Statistics module
 *
 *        Copyright 2006-2007 INRIA - CIRAD - INRA
 *
 *        File author(s): Yann Guédon <yann.guedon@cirad.fr>
 *                        Jean-Baptiste Durand <Jean-Baptiste.Durand@imag.fr>
 *                        Samuel Dufour-Kowalski <samuel.dufour@sophia.inria.fr>
 *                        Christophe Pradal <christophe.prada@cirad.fr>
 *
 *        Distributed under the GPL 2.0 License.
 *        See accompanying file LICENSE.txt or copy at
 *           http://www.gnu.org/licenses/gpl-2.0.txt
 *
 *        OpenAlea WebSite : http://openalea.gforge.inria.fr
 *
 *        $Id: export_tops.cpp 6169 2009-04-01 16:42:59Z cokelaer $
 *
 *-----------------------------------------------------------------------------*/

#include "wrapper_util.h"


#include "stat_tool/stat_tools.h"
#include "stat_tool/distribution.h"
#include "stat_tool/curves.h"
#include "stat_tool/markovian.h"
#include "stat_tool/stat_label.h"
#include "sequence_analysis/sequences.h"
#include "sequence_analysis/variable_order_markov.h"
#include "sequence_analysis/sequence_label.h"
#include "tool/config.h"

#include <boost/python.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/list.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/make_constructor.hpp>

#include "boost_python_aliases.h"

using namespace boost::python;
using namespace boost;
//using namespace stat_tool;

#define WRAP VariableOrderMarkovWrap
class VariableOrderMarkovWrap {

public:

  static void
   file_ascii_write(const Variable_order_markov& d, const char* path, bool exhaustive)
   {
     bool result = true;
     Format_error error;

     result = d.ascii_write(error, path, exhaustive);
     if (!result)
       sequence_analysis::wrap_util::throw_error(error);
   }

  static Parametric_model*
  extract(const Variable_order_markov& input, int type , int variable , int value)
  {
    SIMPLE_METHOD_TEMPLATE_1(input, extract, Parametric_model, type, variable, value);
  }

  static Variable_order_markov_data*
  extract_data(const Variable_order_markov& input)
  {
    SIMPLE_METHOD_TEMPLATE_0(input, extract_data, Variable_order_markov_data);
  }


};

// Boost declaration

void class_variable_order_markov() {

  class_<Variable_order_markov, bases<STAT_interface > >
  ("_Variable_order_markov", "Variable_order_markov\n"
      "constructor(type(char in ['o','e']), nb_state(int), nb_row(int)\n"
      "constructor(type(char in ['o','e']), nb_state(int), nb_row(int), max_order(int)\n"
      "constructor(type(char in ['o','e']), nb_state(int), order(int), init_flag(bool), output_process=0, nb_value=0\n"

  )

      // type = 'o' : ordinaire, or 'e' : en equilibre des probabilites de transition
    .def(init <char, int, int>())
    .def(init <char, int, int,int>())
    .def(init <char, int, int, bool, optional<int, int> >())

    .def(self_ns::str(self)) //__str__

    .def("file_ascii_write", WRAP::file_ascii_write,"Save vector summary into a file")

    .add_property("nb_iterator", &Variable_order_markov::get_nb_iterator, "todo")
    .add_property("max_order", &Variable_order_markov::get_max_order, "todo")
    .add_property("nb_output_process", &Variable_order_markov::get_nb_output_process, "todo")

    .def("get_memory_type", &Variable_order_markov::get_memory_type, args("memory"),"todo")
    .def("get_order", &Variable_order_markov::get_order, args("memory"),"todo")
    .def("get_state", &Variable_order_markov::get_state, args("memory", "lag"),"todo")
    .def("get_parent", &Variable_order_markov::get_parent, args("memory"),"todo")
    .def("get_child", &Variable_order_markov::get_child, args("memory", "state"),"todo")
    .def("get_next", &Variable_order_markov::get_next, args("memory", "state"),"todo")
    .def("get_nb_memory", &Variable_order_markov::get_nb_memory, args("memory"),"todo")
    .def("get_previous", &Variable_order_markov::get_previous,args("memory", "state"), "todo")


    DEF_RETURN_VALUE("extract", &WRAP::extract, args("type","variable","value"), "returns parametric model")
    DEF_RETURN_VALUE_NO_ARGS("extract_data", &WRAP::extract_data, "returns variable_order_markov_data")

;


/*
  Variable_order_markov();
    Variable_order_markov(const Variable_order_markov &markov , int inb_output_process , int nb_value);
    Variable_order_markov(const Variable_order_markov *pmarkov ,  const Nonparametric_process *pobservation , int length);
    Variable_order_markov(const Variable_order_markov &markov , bool data_flag = true)  :Chain(markov) { copy(markov , data_flag); }

    Parametric_model* extract(Format_error &error , int type , int variable , int value) const;
    Variable_order_markov_data* extract_data(Format_error &error) const;
    Variable_order_markov* thresholding(double min_probability = MIN_PROBABILITY) const;

  void characteristic_computation(int length , bool counting_flag , int variable = I_DEFAULT);
  void characteristic_computation(const Variable_order_markov_data &seq , bool counting_flag ,
    int variable = I_DEFAULT , bool length_flag = true);

    Correlation* state_autocorrelation_computation(Format_error &error , int istate ,  int max_lag = MAX_LAG) const;
    Correlation* output_autocorrelation_computation(Format_error &error , int variable ,  int output , int max_lag = MAX_LAG) const;

    double likelihood_computation(const Markovian_sequences &seq , int index) const;
    double likelihood_computation(const Variable_order_markov_data &seq) const;

   Variable_order_markov_data* simulation(Format_error &error , const Histogram &hlength , bool counting_flag = true , bool divergence_flag = false) const;
   Variable_order_markov_data* simulation(Format_error &error , int nb_sequence , int length , bool counting_flag = true) const;
   Variable_order_markov_data* simulation(Format_error &error , int nb_sequence , const Markovian_sequences &iseq , bool counting_flag = true) const;

  Distance_matrix* divergence_computation(Format_error &error , std::ostream &os , int nb_model ,const Variable_order_markov **imarkov , Histogram **hlength ,  const char *path = 0) const;
  Distance_matrix* divergence_computation(Format_error &error , std::ostream &os , int nb_model ,const Variable_order_markov **markov , int nb_sequence ,  int length , const char *path = 0) const;
  Distance_matrix* divergence_computation(Format_error &error , std::ostream &os , int nb_model , const Variable_order_markov **markov , int nb_sequence ,  const Markovian_sequences **seq , const char *path = 0) const;

  Variable_order_markov_data* get_markov_data() const { return markov_data; }
  Nonparametric_sequence_process* get_nonparametric_process(int variable) const{ return nonparametric_process[variable]; }
  Parametric_process** get_parametric_process() const { return parametric_process; }
  Parametric_process* get_parametric_process(int variable)const { return parametric_process[variable]; }
*/




}
#undef WRAP


void class_variable_order_markov_data() {

        class_<Variable_order_markov_data, bases<Markovian_sequences > >
        ("_Variable_order_markov_data", "Variable_order_markov_data")

;
        /*
Variable_order_markov_data();
   Variable_order_markov_data(const Histogram &ihlength , int inb_variable , bool init_flag = false);
   Variable_order_markov_data(const Markovian_sequences &seq);
   Variable_order_markov_data(const Markovian_sequences &seq , char transform ,
                              bool initial_run_flag);
   Variable_order_markov_data(const Variable_order_markov_data &seq , bool model_flag = true ,
                              char transform = 'c')
   :Markovian_sequences(seq , transform) { copy(seq , model_flag); }
   ~Variable_order_markov_data();
   Variable_order_markov_data& operator=(const Variable_order_markov_data &seq);

   Distribution_data* extract(Format_error &error , int type ,
                              int variable , int value) const;
   Variable_order_markov_data* remove_index_parameter(Format_error &error) const;

   Correlation* state_autocorrelation_computation(Format_error &error , int istate ,
                                                  int max_lag = MAX_LAG) const;
   Correlation* output_autocorrelation_computation(Format_error &error , int variable ,
                                                   int output , int max_lag = MAX_LAG) const;

   std::ostream& ascii_data_write(std::ostream &os , char format = 'c' ,
                                  bool exhaustive = false) const;
   bool ascii_data_write(Format_error &error , const char *path ,
                         char format = 'c' , bool exhaustive = false) const;

   std::ostream& ascii_write(std::ostream &os , bool exhaustive = false) const;
   bool ascii_write(Format_error &error , const char *path ,
                    bool exhaustive = false) const;
   bool spreadsheet_write(Format_error &error , const char *path) const;
   bool plot_write(Format_error &error , const char *prefix ,
                   const char *title = 0) const;

   void build_transition_count(const Variable_order_markov &markov ,
                               bool begin = true , bool non_terminal = false);
   void order0_estimation(Variable_order_markov &markov) const;

   // acces membres de la classe

   Variable_order_markov* get_markov() const { return markov; }
   Variable_order_chain_data* get_chain_data() const { return chain_data; }
   double get_likelihood() const { return likelihood; }
   double get_hidden_likelihood() const { return hidden_likelihood; }
   double get_posterior_probability(int index) const { return posterior_probability[index]; }
   */
}


