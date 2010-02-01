/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       V-Plants: Exploring and Modeling Plant Architecture
 *
 *       Copyright 1995-2010 CIRAD/INRIA Virtual Plants
 *
 *       File author(s): Y. Guedon (yann.guedon@cirad.fr)
 *
 *       $Source$
 *       $Id$
 *
 *       Forum for V-Plants developers:
 *
 *  ----------------------------------------------------------------------------
 *
 *                      GNU General Public Licence
 *
 *       This program is free software; you can redistribute it and/or
 *       modify it under the terms of the GNU General Public License as
 *       published by the Free Software Foundation; either version 2 of
 *       the License, or (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public
 *       License along with this program; see the file COPYING. If not,
 *       write to the Free Software Foundation, Inc., 59
 *       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  ----------------------------------------------------------------------------
 */



#include <math.h>
#include <cstdlib>
#include "stat_tools.h"
#include "distribution.h"
#include "stat_label.h"

using namespace std;



/*--------------------------------------------------------------*
 *
 *  Convolution de 2 lois (le resultat de la convolution peut 
 *  etre mis dans l'une des 2 lois).
 *
 *  arguments : references sur les 2 lois, nombre de valeurs
 *              de la loi resultante.
 *
 *--------------------------------------------------------------*/

void Distribution::convolution(Distribution &dist1 , Distribution &dist2 , int inb_value)

{
  register int i , j;
  int coffset , cnb_value , min , max;
  double sum , *pmass1 , *pmass2 , *pmass;


  cnb_value = MIN(dist1.nb_value + dist2.nb_value - 1 , alloc_nb_value);
  if ((inb_value != I_DEFAULT) && (inb_value < cnb_value)) {
    cnb_value = inb_value;
  }
  coffset = MIN(dist1.offset + dist2.offset , cnb_value - 1);

  pmass = mass + cnb_value;

  for (i = cnb_value - 1;i >= coffset;i--) {
    sum = 0.;
    min = MAX(dist1.offset , i - (dist2.nb_value - 1));
    max = MIN(dist1.nb_value - 1 , i - dist2.offset);

    if (max >= min) {
      pmass1 = dist1.mass + min;
      pmass2 = dist2.mass + i - min;

      for (j = min;j <= max;j++) {
        sum += *pmass1++ * *pmass2--;
      }
    }

    *--pmass = sum;
  }

  for (i = coffset - 1;i >= 0;i--) {
    *--pmass = 0.;
  }

  offset = coffset;
  nb_value = cnb_value;
}


/*--------------------------------------------------------------*
 *
 *  Calcul d'une loi binomiale.
 *
 *  arguments : nombre de valeurs,
 *              mode de calcul ('s' : standard, 'r' : renouvellement).
 *
 *--------------------------------------------------------------*/

void Parametric::binomial_computation(int inb_value , char mode)

{
  register int i;
  int set , subset;
  double failure = 1. - probability , success = probability , ratio , scale , term , *pmass;


  switch (mode) {
  case 's' :
    offset = inf_bound;
    nb_value = sup_bound + 1;
    break;
  case 'r' :
    offset = MIN(inf_bound , inb_value - 1);
    nb_value = MIN(sup_bound + 1 , inb_value);
    break;
  }

  // valeurs de probabilite nulle avant la borne inferieure

  pmass = mass;
  for (i = 0;i < MIN(nb_value , inf_bound);i++) {
    *pmass++ = 0.;
  }

  if (nb_value > inf_bound) {
    set = sup_bound - inf_bound;

    if (probability <= B_PROBABILITY) {
      subset = 0;

      // cas calcul direct

      if ((sup_bound - inf_bound) / failure < B_THRESHOLD) {

        // calcul de la probabilite de la borne inferieure

        term = 1.;
        for (i = 0;i < sup_bound - inf_bound;i++) {
          term *= failure;
        }
        *pmass++ = term;

        // calcul des probabilites des valeurs suivantes

        ratio = success / failure;

        for (i = inf_bound + 1;i < nb_value;i++) {
          scale = (double)(set - subset) / (double)(subset + 1);
          subset++;
          term *= scale * ratio;
          *pmass++ = term;
        }
      }

      // cas calcul en log

      else {

        // calcul de la probabilite de la borne inferieure

        term = (sup_bound - inf_bound) * log(failure);
        *pmass++ = exp(term);

        // calcul des probabilites des valeurs suivantes

        ratio = log(success / failure);

        for (i = inf_bound + 1;i < nb_value;i++) {
          scale = (double)(set - subset) / (double)(subset + 1);
          subset++;
          term += log(scale) + ratio;
          *pmass++ = exp(term);
        }
      }
    }

    else {
      pmass = mass + nb_value;
      subset = set - 1;

      // cas calcul direct

      if ((sup_bound - inf_bound) / success < B_THRESHOLD) {

        // calcul de la probabilite de la borne superieure

        term = 1.;
        for (i = 0;i < sup_bound - inf_bound;i++) {
          term *= success;
        }
        if (sup_bound < nb_value) {
          *--pmass = term;
        }

        // calcul des probabilites des valeurs precedentes

        ratio = failure / success;

        for (i = sup_bound - 1;i >= inf_bound;i--) {
          scale = (double)(subset + 1) / (double)(set - subset);
          subset--;
          term *= scale * ratio;
          if (i < nb_value) {
            *--pmass = term;
          }
        }
      }

      // cas calcul en log

      else {

        // calcul de la probabilite de la borne superieure

        term = (sup_bound - inf_bound) * log(success);
        if (sup_bound < nb_value) {
          *--pmass = exp(term);
        }

        // calcul des probabilites des valeurs precedentes

        ratio = log(failure / success);

        for (i = sup_bound - 1;i >= inf_bound;i--) {
          scale = (double)(subset + 1) / (double)(set - subset);
          subset--;
          term += log(scale) + ratio;
          if (i < nb_value) {
            *--pmass = exp(term);
          }
        }
      }
    }
  }

  cumul_computation();
}


/*--------------------------------------------------------------*
 *
 *  Calcul d'une loi de Poisson, loi definie sur
 *  [borne inferieure , ... , infini]. On fixe le nombre de valeurs,
 *  soit a partir d'un seuil calcule sur la fonction de repartition,
 *  soit a partir d'une borne predefinie.
 *
 *  arguments : nombre de valeurs,
 *              seuil sur la fonction de repartition,
 *              mode de calcul ('s' : standard, 'r' : renouvellement).
 *
 *--------------------------------------------------------------*/

void Parametric::poisson_computation(int inb_value , double cumul_threshold ,
                                     char mode)

{
  register int i;
  double log_parameter , num , denom , *pmass , *pcumul;


  pmass = mass;
  pcumul = cumul;

  switch (mode) {

  // cas calcul complet

  case 's' : {

    // valeurs de probabilite nulle avant la borne inferieure

    for (i = 0;i < inf_bound;i++) {
      *pmass++ = 0.;
      *pcumul++ = 0.;
    }

    i = 1;

    // cas calcul direct

    if (parameter < P_THRESHOLD) {

      // calcul de la probabilite de la borne inferieure

      *pmass = exp(-parameter);
      *pcumul = *pmass;

      // calcul des probabilites des valeurs suivantes

      while (((*pcumul < cumul_threshold) || (i + inf_bound < inb_value)) &&
             (i + inf_bound < alloc_nb_value)) {
        pmass++;
        pcumul++;
        *pmass = *(pmass - 1) * parameter / i;
        *pcumul = *(pcumul - 1) + *pmass;
        i++;
      }
    }

    // cas calcul en log

    else {

      // calcul de la probabilite de la borne inferieure

      num = -parameter;
      *pmass = exp(num);
      *pcumul = *pmass;

      // calcul des probabilites des valeurs suivantes

      log_parameter = log(parameter);
      denom = 0.;

      while (((*pcumul < cumul_threshold) || (i + inf_bound < inb_value)) &&
             (i + inf_bound < alloc_nb_value)) {
        num += log_parameter;
        denom += log((double)i);
        *++pmass = exp(num - denom);
        pcumul++;
        *pcumul = *(pcumul - 1) + *pmass;
        i++;
      }
    }

    i += inf_bound;
    break;
  }

  // cas calcul incomplet (renouvellement)

  case 'r' : {

    // valeurs de probabilite nulle avant la borne inferieure

    for (i = 0;i < MIN(inb_value , inf_bound);i++) {
      *pmass++ = 0.;
      *pcumul++ = 0.;
    }

    if (inb_value > inf_bound) {
      i = 1;

      // cas calcul direct

      if (parameter < P_THRESHOLD) {

        // calcul de la probabilite de la borne inferieure

        *pmass = exp(-parameter);
        *pcumul = *pmass;

        // calcul des probabilites des valeurs suivantes

        while ((*pcumul < cumul_threshold) && (i + inf_bound < inb_value)) {
          pmass++;
          pcumul++;
          *pmass = *(pmass - 1) * parameter / i;
          *pcumul = *(pcumul - 1) + *pmass;
          i++;
        }
      }

      // cas calcul en log

      else {

        // calcul de la probabilite de la borne inferieure

        num = -parameter;
        *pmass = exp(num);
        *pcumul = *pmass;

        // calcul des probabilites des valeurs suivantes

        log_parameter = log(parameter);
        denom = 0.;

        while ((*pcumul < cumul_threshold) && (i + inf_bound < inb_value)) {
          num += log_parameter;
          denom += log((double)i);
          *++pmass = exp(num - denom);
          pcumul++;
          *pcumul = *(pcumul - 1) + *pmass;
          i++;
        }
      }

      i += inf_bound;
    }
    break;
  }
  }

  offset = MIN(inf_bound , i - 1);
  nb_value = i;
}


/*--------------------------------------------------------------*
 *
 *  Calcul d'une loi binomiale negative, loi definie sur
 *  [borne inferieure , ... , infini]. On fixe le nombre de valeurs,
 *  soit a partir d'un seuil calcule sur la fonction de repartition,
 *  soit a partir d'une borne predefinie.
 *
 *  arguments : nombre de valeurs, seuil sur la fonction de repartition,
 *              mode de calcul ('s' : standard, 'r' : renouvellement).
 *
 *--------------------------------------------------------------*/

void Parametric::negative_binomial_computation(int inb_value , double cumul_threshold ,
                                               char mode)

{
  register int i;
  double failure = 1. - probability , success = probability , log_failure ,
         set , subset , scale , term , *pmass , *pcumul;


  pmass = mass;
  pcumul = cumul;

  switch (mode) {

    // cas calcul complet

    case 's' : {

    // valeurs de probabilite nulle avant la borne inferieure

    for (i = 0;i < inf_bound;i++) {
      *pmass++ = 0.;
      *pcumul++ = 0.;
    }

    i++;
    subset = parameter - 1.;
    set = subset;

    // cas calcul direct

    if (sqrt(parameter) / success < NB_THRESHOLD) {

      // calcul de la probabilite de la borne inferieure

      term = pow(success , parameter);
      *pmass = term;
      *pcumul = *pmass;

      // calcul des probabilites des valeurs suivantes

      while (((*pcumul < cumul_threshold) || (i < inb_value)) &&
             (i < alloc_nb_value)) {
        set++;
        scale = set / (set - subset);
        term *= scale * failure;
        *++pmass = term;
        pcumul++;
        *pcumul = *(pcumul - 1) + *pmass;
        i++;
      }
    }

    // cas calcul en log

    else {

      // calcul de la probabilite de la borne inferieure

      term = parameter * log(success);
      *pmass = exp(term);
      *pcumul = *pmass;

      // calcul des probabilites des valeurs suivantes

      log_failure = log(failure);

      while (((*pcumul < cumul_threshold) || (i < inb_value)) &&
             (i < alloc_nb_value)) {
        set++;
        scale = set / (set - subset);
        term += log(scale) + log_failure;
        *++pmass = exp(term);
        pcumul++;
        *pcumul = *(pcumul - 1) + *pmass;
        i++;
      }
    }
    break;
  }

  // cas calcul incomplet (renouvellement)

  case 'r' : {

    // valeurs de probabilite nulle avant la borne inferieure

    for (i = 0;i < MIN(inf_bound , inb_value);i++) {
      *pmass++ = 0.;
      *pcumul++ = 0.;
    }

    if (inb_value > inf_bound) {
      i++;
      subset = parameter - 1.;
      set = subset;

      // cas calcul direct

      if (sqrt(parameter) / success < NB_THRESHOLD) {

        // calcul de la probabilite de la borne inferieure

        term = pow(success , parameter);
        *pmass = term;
        *pcumul = *pmass;

        // calcul des probabilites de valeurs suivantes

        while ((*pcumul < cumul_threshold) && (i < inb_value)) {
          set++;
          scale = set / (set - subset);
          term *= scale * failure;
          *++pmass = term;
          pcumul++;
          *pcumul = *(pcumul - 1) + *pmass;
          i++;
        }
      }

      // cas calcul en log

      else {

        // calcul de la probabilite de la borne inferieure

        term = parameter * log(success);
        *pmass = exp(term);
        *pcumul = *pmass;

        // calcul des probabilites des valeurs suivantes

        log_failure = log(failure);

        while ((*pcumul < cumul_threshold) && (i < inb_value)) {
          set++;
          scale = set / (set - subset);
          term += log(scale) + log_failure;
          *++pmass = exp(term);
          pcumul++;
          *pcumul = *(pcumul - 1) + *pmass;
          i++;
        }
      }
    }
    break;
  }
  }

  offset = MIN(inf_bound , i - 1);
  nb_value = i;
}


/*--------------------------------------------------------------*
 *
 *  Calcul d'une loi uniforme.
 *
 *--------------------------------------------------------------*/

void Parametric::uniform_computation()

{
  register int i;
  double proba , *pmass;


  offset = inf_bound;
  nb_value = sup_bound + 1;

  pmass = mass;
  for (i = 0;i < inf_bound;i++) {
    *pmass++ = 0.;
  }

  proba = 1. / (double)(sup_bound - inf_bound + 1);
  for (i = inf_bound;i <= sup_bound;i++) {
    *pmass++ = proba;
  }

  cumul_computation();
}


/*--------------------------------------------------------------*
 *
 *  Calcul du nombre de valeurs d'une loi discrete elementaire.
 *
 *  arguments : identificateur, bornes inferieure et superieure,
 *              parametre, probabilite,
 *              seuil sur la fonction de repartition.
 *
 *--------------------------------------------------------------*/

int nb_value_computation(int ident , int inf_bound , int sup_bound ,
                         double parameter , double probability , double cumul_threshold)

{
  int nb_value = 0;


  if ((ident == BINOMIAL) || (ident == UNIFORM)) {
    nb_value = sup_bound + 1;
  }

  else {
    if ((ident == POISSON) || (ident == NEGATIVE_BINOMIAL)) {
      Parametric *dist;

      dist = new Parametric(ident , inf_bound , sup_bound , parameter , probability ,
                            cumul_threshold);
      nb_value = dist->nb_value;
      delete dist;
    }
  }

  return nb_value;
}


/*--------------------------------------------------------------*
 *
 *  Calcul d'une loi discrete elementaire (binomiale, Poisson,
 *  binomiale negative, ou uniforme).
 *
 *  arguments : nombre minimum de valeurs,
 *              seuil sur la fonction de repartition.
 *
 *--------------------------------------------------------------*/

void Parametric::computation(int min_nb_value , double cumul_threshold)

{
  if (ident > 0) {
    switch (ident) {
    case BINOMIAL :
      binomial_computation(1 , 's');
      break;
    case POISSON :
      poisson_computation(min_nb_value , cumul_threshold , 's');
      break;
    case NEGATIVE_BINOMIAL :
      negative_binomial_computation(min_nb_value , cumul_threshold , 's');
      break;
    case UNIFORM :
      uniform_computation();
      break;
    }

    max_computation();
    mean_computation();
    variance_computation();
  }
}


/*--------------------------------------------------------------*
 *
 *  Calcul de la loi de l'intervalle de temps residuel a partir
 *  de la loi de temps de retour/sejour.
 *
 *  argument : loi de temps de retour/sejour.
 *
 *--------------------------------------------------------------*/

void Forward::computation(const Parametric &dist)

{
  register int i;
  double norm , *pmass , *icumul;


  pmass = mass;
  icumul = dist.cumul;

  offset = 1;
  nb_value = dist.nb_value;
  *pmass++ = 0.;

  // calcul de la quantite de normalisation

  if (ident == NONPARAMETRIC) {
    norm = dist.mean;
  }
  else {
    norm = parametric_mean_computation();
  }

  // calcul des probabilites des valeurs

  for (i = 1;i < nb_value;i++) {
    *pmass++ = (1. - *icumul++) / norm;
  }

  cumul_computation();

  max_computation();
  mean_computation();
  variance_computation();
}


/*--------------------------------------------------------------*
 *
 *  Calcul de la vraisemblance d'un histogramme pour la fonction de
 *  survie d'une loi donnee.
 *
 *  argument : reference sur un objet Histogram.
 *
 *--------------------------------------------------------------*/

double Distribution::survivor_likelihood_computation(const Histogram &histo) const

{
  register int i;
  int *pfrequency;
  double likelihood = 0. , *pcumul;


  if (histo.nb_element > 0) {
    if ((histo.offset == 0) || (histo.nb_value > nb_value)) {
      likelihood = D_INF;
    }

    else {
      pfrequency = histo.frequency + histo.offset;
      pcumul = cumul + histo.offset - 1;

      for (i = histo.offset;i < histo.nb_value;i++) {
        if (*pfrequency > 0) {
          if (*pcumul < 1.) {
            likelihood += *pfrequency * log(1. - *pcumul);
          }
          else {
            likelihood = D_INF;
            break;
          }
        }

        pfrequency++;
        pcumul++;
      }
    }
  }

  return likelihood;
}


/*--------------------------------------------------------------*
 *
 *  Calcul de la valeur du chi2 pour une loi.
 *
 *  argument : reference sur un objet Histogram.
 *
 *--------------------------------------------------------------*/

double Distribution::chi2_value_computation(const Histogram &histo) const

{
  register int i;
  int *pfrequency;
  double value , var1 , var2 , *pmass;


  if ((histo.offset < offset) || (histo.nb_value > nb_value)) {
    value = -D_INF;
  }

  else {
    pfrequency = histo.frequency + offset;
    pmass = mass + offset;
    value = 0.;

    for (i = offset;i < histo.nb_value;i++) {
      if (*pmass > 0.) {
        var1 = histo.nb_element * *pmass;
        if (cumul[nb_value - 1] < CUMUL_THRESHOLD) {
          var1 /= cumul[nb_value - 1];
        }

        var2 = *pfrequency - var1;
        value += var2 * var2 / var1;
      }

      else {
        if (*pfrequency > 0) {
          value = -D_INF;
          break;
        }
      }

      pfrequency++;
      pmass++;
    }

    if (value != -D_INF) {
      var1 = 0.;
      for (i = histo.nb_value;i < nb_value;i++) {
        var1 += histo.nb_element * *pmass++;
      }
      if (cumul[nb_value - 1] < CUMUL_THRESHOLD) {
        var1 /= cumul[nb_value - 1];
      }
      value += var1;
    }
  }

  return value;
}


/*--------------------------------------------------------------*
 *
 *  Regroupement des valeurs, calcul du nombre de degres de liberte et
 *  de la valeur du chi2.
 *
 *  arguments : reference sur un objet Histogram et sur un objet Test.
 *
 *--------------------------------------------------------------*/

void Distribution::chi2_degree_of_freedom(const Histogram &histo , Test &test) const

{
  register int i , j;
  int *pfrequency , *filter_frequency;
  double *pmass , *pcumul , *filter_mass;
  Distribution *filter_dist;
  Histogram *filter_histo;


  if ((histo.offset >= offset) && (histo.nb_value <= nb_value)) {

    // creation et initialisation des objets Distribution et Histogram

    filter_dist = new Distribution(nb_value);
    filter_dist->offset = offset;
    filter_dist->nb_parameter = nb_parameter;

    filter_histo = new Histogram(histo.nb_value);
    filter_histo->nb_element = histo.nb_element;

    // regroupement des valeurs

    pmass = mass + offset;
    pcumul = cumul + offset;
    pfrequency = histo.frequency + offset;
    filter_mass = filter_dist->mass + offset;
    filter_frequency = filter_histo->frequency + offset;
    *filter_mass = 0.;
    *filter_frequency = 0;
    j = offset + 1;

    for (i = offset;i < histo.nb_value - 1;i++) {
      *filter_mass += *pmass++;
      *filter_frequency += *pfrequency++;
      if ((*filter_mass * histo.nb_element / cumul[nb_value - 1] > CHI2_FREQUENCY) &&
          ((1. - *pcumul) * histo.nb_element / cumul[nb_value - 1] > 1)) {
        *++filter_mass = 0.;
        *++filter_frequency = 0;
        j++;
      }
      pcumul++;
    }
    *filter_frequency += *pfrequency;

    filter_histo->offset_computation();
    filter_histo->nb_value = j;

    for (i = histo.nb_value - 1;i < nb_value - 1;i++) {
      *filter_mass += *pmass++;
      if ((*filter_mass * histo.nb_element / cumul[nb_value - 1] > CHI2_FREQUENCY) &&
          ((1. - *pcumul) * histo.nb_element / cumul[nb_value - 1] > 1)) {
        *++filter_mass = 0.;
        j++;
      }
      pcumul++;
    }
    *filter_mass += *pmass;

    filter_dist->nb_value = j;
    filter_dist->cumul_computation();

    // mise a jour du nombre de degres de libertes et calcul de
    // la valeur du chi2

    test.df1 = filter_dist->nb_value - filter_dist->offset -
               filter_dist->nb_parameter - 1;
    if (test.df1 < 1) {
      test.df1 = 1;
    }

    test.value = filter_dist->chi2_value_computation(*filter_histo);

#   ifdef DEBUG
/*    cout << *filter_histo;
    cout << *filter_dist; */
#   endif

    delete filter_dist;
    delete filter_histo;
  }
}


/*--------------------------------------------------------------*
 *
 *  Test de l'ajustement d'une loi a un histogramme.
 *
 *  arguments : reference sur un objet Histogram et sur un objet Test.
 *
 *--------------------------------------------------------------*/

void Distribution::chi2_fit(const Histogram &histo , Test &test) const

{
  if ((histo.offset >= offset) && (histo.nb_value <= nb_value)) {
    chi2_degree_of_freedom(histo , test);
    if ((test.df1 > 0) && (test.value > 0.)) {
      test.chi2_critical_probability_computation();
    }
  }
}


/*--------------------------------------------------------------*
 *
 *  Troncature d'une loi.
 *
 *  arguments : reference sur un objet Format_error, valeur maximum.
 *
 *--------------------------------------------------------------*/

Parametric_model* Distribution::truncate(Format_error &error , int imax_value) const

{
  register int i;
  double *pmass , *pcumul , *cmass , *ccumul;
  Parametric_model *dist;


  error.init();

  if (imax_value <= offset) {
    dist = NULL;
    error.update(STAT_error[STATR_MAX_VALUE]);
  }

  else {

    // creation d'un objet Parametric_model

    dist = new Parametric_model(MIN(imax_value + 1 , nb_value));

    pmass = dist->mass;
    cmass = mass;
    pcumul = dist->cumul;
    ccumul = cumul;

    for (i = 0;i < dist->nb_value - 1;i++) {
      *pmass++ = *cmass++;
      *pcumul++ = *ccumul++;
    }
    *pmass = 1. - *(pcumul - 1);
    *pcumul = 1.;

    dist->offset = offset;
    dist->max_computation();
    dist->mean_computation();
    dist->variance_computation();
  }

  return dist;
}


/*--------------------------------------------------------------*
 *
 *  Ajustement d'une loi a un histogramme.
 *
 *  arguments : references sur un objet Format_error et
 *              sur un objet Parametric.
 *
 *--------------------------------------------------------------*/

Parametric_model* Histogram::fit(Format_error &error , const Parametric &idist) const

{
  Parametric_model *dist;


  error.init();

  if ((offset < idist.offset) || (nb_value > idist.nb_value)) {
    dist = NULL;
    error.update(STAT_error[STATR_VALUE_RANGE]);
  }

  else {
    dist = new Parametric_model(idist , this);
  }

  return dist;
}


/*--------------------------------------------------------------*
 *
 *  Estimation des parametres d'une loi discrete elementaire
 *  (binomiale negative, binomiale, Poisson) a partir d'un histogramme.
 *
 *  arguments : identificateur de la loi, borne inferieure minimum,
 *              flag sur la borne inferieure, seuil sur la fonction de repartition.
 *
 *--------------------------------------------------------------*/

Parametric* Histogram::parametric_estimation(int ident , int min_inf_bound ,
                                             bool flag , double cumul_threshold) const

{
  double likelihood;
  Parametric *dist;


  // creation d'un objet Parametric

  dist = new Parametric((int)(nb_value * SAMPLE_NB_VALUE_COEFF) , ident);

  // estimation des parametres de la loi

  likelihood = Reestimation<int>::parametric_estimation(dist , min_inf_bound ,
                                                        flag , cumul_threshold);

  // mise a jour de la loi estimee

  if (likelihood != D_INF) {
    dist->computation(nb_value , cumul_threshold);
  }
  else {
    delete dist;
    dist = NULL;
  }

  return dist;
}


/*--------------------------------------------------------------*
 *
 *  Estimation des parametres d'une loi discrete elementaire
 *  (binomiale negative, binomiale, Poisson) a partir d'un histogramme.
 *
 *  arguments : reference sur un objet Format_error, identificateur de la loi,
 *              borne inferieure minimum, flag sur la borne inferieure,
 *              seuil sur la fonction de repartition.
 *
 *--------------------------------------------------------------*/

Parametric_model* Histogram::parametric_estimation(Format_error &error , int ident ,
                                                   int min_inf_bound , bool flag ,
                                                   double cumul_threshold) const

{
  double likelihood;
  Parametric_model *dist;


  error.init();

  if ((min_inf_bound < 0) || (min_inf_bound > 1) || (min_inf_bound > offset)) {
    dist = NULL;
    error.update(STAT_error[STATR_MIN_INF_BOUND]);
  }

  else {

    // creation d'un objet Parametric_model

    dist = new Parametric_model((int)(nb_value * SAMPLE_NB_VALUE_COEFF) , ident);
    dist->histogram = new Distribution_data(*this);

    // estimation des parametres de la loi

    likelihood = Reestimation<int>::parametric_estimation(dist , min_inf_bound ,
                                                          flag , cumul_threshold);

    if (likelihood != D_INF) {

      // mise a jour de la loi estimee

      dist->computation(nb_value , cumul_threshold);

      // mise a jour du nombre de parametres inconnus

      dist->nb_parameter_update();
      if (!flag) {
        (dist->nb_parameter)--;
      }
    }

    else {
      delete dist;
      dist = NULL;
      error.update(STAT_error[STATR_ESTIMATION_FAILURE]);
    }
  }

  return dist;
}


/*--------------------------------------------------------------*
 *
 *  Estimation des parametres d'une loi discrete elementaire
 *  (binomiale negative, binomiale, Poisson) a partir d'un histogramme.
 *
 *  arguments : reference sur un objet Format_error, borne inferieure minimum,
 *              flag sur la borne inferieure, seuil sur la fonction de repartition.
 *
 *--------------------------------------------------------------*/

Parametric_model* Histogram::type_parametric_estimation(Format_error &error ,
                                                        int min_inf_bound , bool flag ,
                                                        double cumul_threshold) const

{
  double likelihood;
  Parametric_model *dist;


  error.init();

  if ((min_inf_bound < 0) || (min_inf_bound > 1) || (min_inf_bound > offset)) {
    dist = NULL;
    error.update(STAT_error[STATR_MIN_INF_BOUND]);
  }

  else {

    // creation d'un objet Parametric_model

    dist = new Parametric_model((int)(nb_value * SAMPLE_NB_VALUE_COEFF));
    dist->histogram = new Distribution_data(*this);

    // estimation des parametres de la loi

    likelihood = Reestimation<int>::type_parametric_estimation(dist , min_inf_bound ,
                                                               flag , cumul_threshold);

    if (likelihood != D_INF) {

      // mise a jour de la loi estimee

      dist->computation(nb_value , cumul_threshold);

      // mise a jour du nombre de parametres inconnus

      dist->nb_parameter_update();
      if (!flag) {
        (dist->nb_parameter)--;
      }
    }

    else {
      delete dist;
      dist = NULL;
      error.update(STAT_error[STATR_ESTIMATION_FAILURE]);
    }
  }

  return dist;
}


/*--------------------------------------------------------------*
 *
 *  Calcul des termes de penalisation dans le cadre d'une approche
 *  de penalisation de la vraisemblance.
 *
 *  arguments : poids de la penalisation, type de penalisation (difference 1ere,
 *              seconde ou entropy), type de gestion des effets de bord
 *              (zero a l'exterieur du support ou prolongation de la loi).
 *
 *--------------------------------------------------------------*/

void Distribution::penalty_computation(double weight , int type , double *penalty , int outside) const

{
  register int i;
  double *ppenalty;


  ppenalty = penalty + offset;

  switch (type) {

  case FIRST_DIFFERENCE : {
    switch (outside) {
    case ZERO :
      *ppenalty++ = 2 * weight * (2 * mass[offset] - mass[offset + 1]);
      break;
    case CONTINUATION :
      *ppenalty++ = 2 * weight * (mass[offset] - mass[offset + 1]);
      break;
    }

    for (i = offset + 1;i < nb_value - 1;i++) {
      *ppenalty++ = 2 * weight * (-mass[i - 1] + 2 * mass[i] - mass[i + 1]);
    }

    switch (outside) {
    case ZERO :
      *ppenalty = 2 * weight * (-mass[nb_value - 2] + 2 * mass[nb_value - 1]);
      break;
    case CONTINUATION :
      *ppenalty = 2 * weight * (-mass[nb_value - 2] + mass[nb_value - 1]);
      break;
    }
    break;
  }

  case SECOND_DIFFERENCE : {
    i = offset;

    switch (outside) {

    case ZERO : {
      *ppenalty++ = 2 * weight * (6 * mass[i] - 4 * mass[i + 1] + mass[i + 2]);
      i++;
      *ppenalty++ = 2 * weight * (-4 * mass[i - 1] + 6 * mass[i] - 4 * mass[i + 1] +
                                  mass[i + 2]);
      break;
    }

    case CONTINUATION : {
      *ppenalty++ = 2 * weight * (3 * mass[i] - 4 * mass[i + 1] + mass[i + 2]);
      i++;
      *ppenalty++ = 2 * weight * (-3 * mass[i - 1] + 6 * mass[i] - 4 * mass[i + 1] +
                                  mass[i + 2]);
      break;
    }
    }

    for (i = offset + 2;i < nb_value - 2;i++) {
      *ppenalty++ = 2 * weight * (mass[i - 2] - 4 * mass[i - 1] + 6 * mass[i] -
                                  4 * mass[i + 1] + mass[i + 2]);
    }

    i = nb_value - 2;

    switch (outside) {

    case ZERO : {
      *ppenalty++ = 2 * weight * (mass[i - 2] - 4 * mass[i - 1] + 6 * mass[i] -
                                  4 * mass[i + 1]);
      i++;
      *ppenalty = 2 * weight * (mass[i - 2] - 4 * mass[i - 1] + 6 * mass[i]);
      break;
    }

    case CONTINUATION : {
      *ppenalty++ = 2 * weight * (mass[i - 2] - 4 * mass[i - 1] + 6 * mass[i] -
                                  3 * mass[i + 1]);
      i++;
      *ppenalty = 2 * weight * (mass[i - 2] - 4 * mass[i - 1] + 3 * mass[i]);
      break;
    }
    }
    break;
  }

  case ENTROPY : {
    for (i = offset;i < nb_value;i++) {
      *ppenalty++ = weight * (log(mass[i]) + 1);
    }
    break;
  }
  }
}


/*--------------------------------------------------------------*
 *
 *  Reestimation des parametres d'une loi discrete elementaire
 *  (binomiale, Poisson, binomiale negative).
 *
 *  arguments : reference sur les quantites de reestimation,
 *              nombre de parametres reestimes (binomiale negative).
 *
 *--------------------------------------------------------------*/

void Parametric::reestimation(const Reestimation<double> *reestim , int nb_estim)

{
  switch (ident) {

  case BINOMIAL : {
    probability = (reestim->mean - inf_bound) / (sup_bound - inf_bound);
    break;
  }

  case POISSON : {
    parameter = reestim->mean - inf_bound;
    break;
  }

  case NEGATIVE_BINOMIAL : {
    switch (nb_estim) {

    case 1 : {
      if (reestim->mean - inf_bound + parameter > 0.) {
        probability = parameter / (reestim->mean - inf_bound + parameter);
      }
      break;
    }

    case 2 : {
/*      register int i;
      double previous_parameter = parameter , sum1 , sum2 , *rfrequency; */

      parameter = (reestim->mean - inf_bound) * probability / (1. - probability);

/*      rfrequency = reestim->frequency + inf_bound + 1;
      sum1 = 0.;
      sum2 = 0.;
      for (i = inf_bound + 1;i < nb_value;i++) {
        sum2 += 1. / (i - inf_bound + previous_parameter - 1);
        sum1 += *rfrequency++ * sum2;
      }

      probability = exp(-sum1 / reestim->nb_element); */

#     ifdef DEBUG
//      cout << "<" << probability << "> ";
#     endif
      break;
    }
    }

    break;
  }
  }
}


/*--------------------------------------------------------------*
 *
 *  Simulation par une loi discrete en utilisant la fonction de repartition.
 *  On teste le terme median de la fonction de repartition pour savoir
 *  ou commencer la recherche.
 *
 *  arguments : nombre de valeurs, pointeur sur la fonction de repartition,
 *              facteur d'echelle.
 *
 *--------------------------------------------------------------*/

int cumul_method(int nb_value , const double *cumul , double scale)

{
  register int i;
  double limit;


  limit = ((double)rand() / (RAND_MAX + 1.)) * scale;
//  limit = ((double)random() / (double)0x7fffffff) * scale;

  if ((limit < cumul[nb_value / 2])) {
    i = 0;
    while (*cumul++ <= limit) {
      i++;
    }
  }

  else {
    i = nb_value - 1;
    cumul += nb_value - 1;
    while (*--cumul > limit) {
      i--;
    }
  }

  return i;
}


/*--------------------------------------------------------------*
 *
 *  Simulation par une loi discrete en utilisant la fonction de repartition
 *
 *--------------------------------------------------------------*/

int Distribution::simulation() const

{
  int value;


  value = offset + cumul_method(nb_value - offset , cumul + offset , 1. - complement);

  return value;
}


/*--------------------------------------------------------------*
 *
 *  Simulation par une loi discrete par la methode du rejet.
 *  Principe : on tire un point (x,Px) dans le rectangle
 *  [xmin,xmax] [0.,Pmax]. Si le point est sous la courbe, on
 *  garde la realisation correspondante (abscisse x), sinon,
 *  on retire un nouveau point.
 *
 *--------------------------------------------------------------*/

int Parametric::simulation() const

{
  int range = nb_value - offset , value;
  double x , y;


  if ((ident == NONPARAMETRIC) || (range < MIN_RANGE) || (range * max > MAX_SURFACE)) {
    value = Distribution::simulation();
  }

  else {
    do {
      x = (double)rand() / (RAND_MAX + 1.);
      y = (double)rand() / (RAND_MAX + 1.);
//      x = (double)random() / (double)0x7fffffff;
//      y = (double)random() / (double)0x7fffffff;
      value = (int)(offset + range * x);
    }
    while (y * max > mass[value]);
  }

  return value;
}


/*--------------------------------------------------------------*
 *
 *  Constitution d'un histogramme par simulation d'une loi parametrique.
 *
 *  arguments : reference sur un objet Format_error, effectif.
 *
 *--------------------------------------------------------------*/

Distribution_data* Parametric_model::simulation(Format_error &error , int nb_element) const

{
  register int i;
  Distribution_data *histo;


  error.init();

  if ((nb_element < 1) || (nb_element > DIST_NB_ELEMENT)) {
    histo = NULL;
    error.update(STAT_error[STATR_SAMPLE_SIZE]);
  }

  else {

    // creation de l'histogramme

    histo = new Distribution_data(*this);
    histo->distribution = new Parametric_model(*this , false);

    // simulation

    for (i = 0;i < nb_element;i++) {
      (histo->frequency[Parametric::simulation()])++;
    }

    // extraction des caracteristiques de l'histogramme

    histo->nb_value_computation();
    histo->offset_computation();
    histo->nb_element = nb_element;
    histo->max_computation();
    histo->mean_computation();
    histo->variance_computation();
  }

  return histo;
}
