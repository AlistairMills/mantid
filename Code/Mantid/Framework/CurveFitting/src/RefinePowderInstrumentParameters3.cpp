/*WIKI*
This algorithm refines the instrumental geometry parameters for powder diffractomers. 
The parameters that can be refined are Dtt1, Zero, Dtt1t, Dtt2t, Zerot, Width and Tcross. 

It serves as the second step to fit/refine instrumental parameters that will be 
introduced in Le Bail Fit. 
It uses the outcome from algorithm FitPowderDiffPeaks(). 

== Mathematics ==
The function to fit is 

TOF_h = n(Zero + Dtt1*d) + (1-n)(Zerot + Dtt1t*d + Dtt2t/d)
n = 1/2 erfc(W*(1-Tcross/d))

The coefficients in this function are strongly correlated to each other. 

== Refinement Algorithm ==
Two refinement algorithms, DirectFit and MonteCarlo, are provided. 

==== DirectFit ====
This is a simple one step fitting.  
If there is one parameter to fit, Levenberg Marquart minimizer is chosen. 
As its coefficients are strongly correlated to each other, Simplex minimizer is used if there are more than 1 parameter to fit. 

==== MonteCarlo ====
This adopts the concept of Monte Carlo random walk in the parameter space. 
In each MC step, one parameter will be chosen, and a new value is proposed for it. 
A constraint fitting by Simplex minimizer is used to fit the coefficients in new configuration. 

Simulated annealing will be tried as soon as it is implemented in Mantid. 

==== Constraint ====

== How to use algorithm with other algorithms ==
This algorithm is designed to work with other algorithms to do Le Bail fit.  The introduction can be found in the wiki page of [[LeBailFit]]. 

*WIKI*/
#include "MantidCurveFitting/RefinePowderInstrumentParameters3.h"

#include "MantidAPI/Axis.h"
#include "MantidAPI/TextAxis.h"
#include "MantidAPI/TableRow.h"
#include "MantidKernel/ListValidator.h"

namespace Mantid
{
namespace CurveFitting
{

  DECLARE_ALGORITHM(RefinePowderInstrumentParameters3)

  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  RefinePowderInstrumentParameters3::RefinePowderInstrumentParameters3()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  RefinePowderInstrumentParameters3::~RefinePowderInstrumentParameters3()
  {
  }

  //----------------------------------------------------------------------------------------------
  /** Set up documention
    */
  void RefinePowderInstrumentParameters3::initDocs()
  {
    setWikiSummary("Refine the instrument geometry related parameters for powder diffractomer. ");
    setOptionalMessage("Parameters include Dtt1, Dtt1t, Dtt2t, Zero, Zerot. ");
  }

  //----------------------------------------------------------------------------------------------
  /** Declare properties
    */
  void RefinePowderInstrumentParameters3::init()
  {
    // Peak position workspace
    declareProperty(new WorkspaceProperty<Workspace2D>("InputPeakPositionWorkspace", "Anonymous",
                                                       Direction::Input),
                    "Data workspace containing workspace positions in TOF agains dSpacing.");

    // Workspace Index
    declareProperty("WorkspaceIndex", 0,
                    "Workspace Index of the peak positions in PeakPositionWorkspace.");

    // Output workspace
    declareProperty(new WorkspaceProperty<Workspace2D>("OutputPeakPositionWorkspace", "Anonymous2",
                                                       Direction::Output),
                    "Output data workspace containing refined workspace positions in TOF agains dSpacing.");

    // Input Table workspace containing instrument profile parameters
    declareProperty(new WorkspaceProperty<TableWorkspace>("InputInstrumentParameterWorkspace", "Anonymous3",
                                                          Direction::Input),
                    "INput tableWorkspace containg instrument's parameters.");

    // Output table workspace containing the refined parameters
    declareProperty(new WorkspaceProperty<TableWorkspace>("OutputInstrumentParameterWorkspace", "Anonymous4",
                                                          Direction::Output),
                    "Output tableworkspace containing instrument's fitted parameters. ");

    // Refinement algorithm
    vector<string> algoptions;
    algoptions.push_back("OneStepFit");
    algoptions.push_back("MonteCarlo");
    auto validator = boost::make_shared<Kernel::StringListValidator>(algoptions);
    declareProperty("RefinementAlgorithm", "MonteCarlo", validator,
                    "Algorithm to refine the instrument parameters.");

    // Random walk steps
    declareProperty("RandomWalkSteps", 10000, "Number of Monte Carlo random walk steps. ");

    // Random seed
    declareProperty("MonteCarloRandomSeed", 0, "Random seed for Monte Carlo simulation. ");

    // Method to calcualte the standard error of peaks
    vector<string> stdoptions;
    stdoptions.push_back("ConstantValue");
    stdoptions.push_back("UseInputValue");
    auto listvalidator = boost::make_shared<Kernel::StringListValidator>(stdoptions);
    declareProperty("StandardError", "ConstantValue", listvalidator,
                    "Algorithm to calculate the standard error of peak positions.");

    // Damping factor
    declareProperty("Damping", 1.0, "Damping factor for (1) minimizer 'damping'. (2) Monte Calro. ");

    // Anealing temperature
    declareProperty("AnnealingTemperature", 1.0, "Starting aneealing temperature.");

    // Monte Carlo iterations
    declareProperty("MonteCarloIterations", 100, "Number of iterations in Monte Carlo random walk.");

    // Output
    declareProperty("ChiSquare", DBL_MAX, Direction::Output);

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Main execution body
    */
  void RefinePowderInstrumentParameters3::exec()
  {
    // 1. Process input
    processInputProperties();

    // 2. Parse input table workspace
    parseTableWorkspaces();

    // 3. Set up main function for peak positions
    ThermalNeutronDtoTOFFunction rawfunc;
    m_positionFunc = boost::make_shared<ThermalNeutronDtoTOFFunction>(rawfunc);
    m_positionFunc->initialize();

    // 3. Fit
    // a) Set up parameter value
    setFunctionParameterValues(m_positionFunc, m_profileParameters);

    // b) Generate some global useful value and Calculate starting chi^2
    API::FunctionDomain1DVector domain(m_dataWS->readX(m_wsIndex));
    API::FunctionValues rawvalues(domain);
    m_positionFunc->function(domain, rawvalues);

    // d) Calcualte statistic
    double startchi2 = calculateFunctionError(m_positionFunc, m_dataWS, m_wsIndex);

    // b) Fit by type
    double finalchi2 = DBL_MAX;
    switch (m_fitMode)
    {
      case FIT:
        // Fit by non-Monte Carlo method
        g_log.notice("Fit by non Monte Carlo algorithm. ");
        finalchi2 = execFitParametersNonMC();
        break;

      case MONTECARLO:
        // Fit by Monte Carlo method
        g_log.notice("Fit by Monte Carlo algorithm.");
        finalchi2 = execFitParametersMC();
        break;

      default:
        // Unsupported
        throw runtime_error("Unsupported fit mode.");
        break;
    }

    // 4. Process the output
    TableWorkspace_sptr fitparamtable = genOutputProfileTable(m_profileParameters,
                                                              startchi2, finalchi2);
    setProperty("OutputInstrumentParameterWorkspace", fitparamtable);

    Workspace2D_sptr outdataws = genOutputWorkspace(domain, rawvalues);
    setProperty("OutputPeakPositionWorkspace", outdataws);

    setProperty("ChiSquare", finalchi2);

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Process input properties
    */
  void RefinePowderInstrumentParameters3::processInputProperties()
  {
    // Data Workspace
    m_dataWS = getProperty("InputPeakPositionWorkspace");

    m_wsIndex = getProperty("WorkspaceIndex");
    if (m_wsIndex < 0 || m_wsIndex >= static_cast<int>(m_dataWS->getNumberHistograms()))
    {
      throw runtime_error("Input workspace index is out of range.");
    }

    // Parameter TableWorkspace
    m_paramTable = getProperty("InputInstrumentParameterWorkspace");

    // Fit mode
    string fitmode = getProperty("RefinementAlgorithm");
    if (fitmode.compare("OneStepFit") == 0)
      m_fitMode = FIT;
    else if (fitmode.compare("MonteCarlo") == 0)
      m_fitMode = MONTECARLO;
    else
    {
      m_fitMode = FIT;
      throw runtime_error("Input RefinementAlgorithm is not supported.");
    }

    // Stanard error mode
    string stdmode = getProperty("StandardError");
    if (stdmode.compare("ConstantValue") == 0)
      m_stdMode = CONSTANT;
    else if (stdmode.compare("UseInputValue") == 0)
      m_stdMode = USEINPUT;
    else
    {
      m_stdMode = USEINPUT;
      throw runtime_error("Input StandardError (mode) is not supported.");
    }

    // Monte Carlo
    m_numWalkSteps = getProperty("RandomWalkSteps");
    if (m_numWalkSteps <= 0)
      throw runtime_error("Monte Carlo walk steps cannot be less or equal to 0. ");

    m_randomSeed = getProperty("MonteCarloRandomSeed");

    m_dampingFactor = getProperty("Damping");

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Parse TableWorkspaces
    */
  void RefinePowderInstrumentParameters3::parseTableWorkspaces()
  {
    m_profileParameters.clear();

    parseTableWorkspace(m_paramTable, m_profileParameters);

  }

  //----------------------------------------------------------------------------------------------
  /** Parse table workspace to a map of Parameters
    */
  void RefinePowderInstrumentParameters3::parseTableWorkspace(TableWorkspace_sptr tablews,
                                                              map<string, Parameter>& parammap)
  {
    // 1. Process Table column names
    std::vector<std::string> colnames = tablews->getColumnNames();
    map<string, size_t> colnamedict;
    convertToDict(colnames, colnamedict);

    int iname = getStringIndex(colnamedict, "Name");
    int ivalue = getStringIndex(colnamedict, "Value");
    int ifit = getStringIndex(colnamedict, "FitOrTie");
    int imin = getStringIndex(colnamedict, "Min");
    int imax = getStringIndex(colnamedict, "Max");
    int istep = getStringIndex(colnamedict, "StepSize");

    if (iname < 0 || ivalue < 0 || ifit < 0)
      throw runtime_error("TableWorkspace does not have column Name, Value and/or Fit.");

    // 3. Parse
    size_t numrows = tablews->rowCount();
    for (size_t irow = 0; irow < numrows; ++irow)
    {
      string parname = tablews->cell<string>(irow, iname);
      double parvalue = tablews->cell<double>(irow, ivalue);
      string fitq = tablews->cell<string>(irow, ifit);

      double minvalue;
      if (imin >= 0)
        minvalue = tablews->cell<double>(irow, imin);
      else
        minvalue = -DBL_MAX;

      double maxvalue;
      if (imax >= 0)
        maxvalue = tablews->cell<double>(irow, imax);
      else
        maxvalue = DBL_MAX;

      double stepsize;
      if (istep >= 0)
        stepsize = tablews->cell<double>(irow, istep);
      else
        stepsize = 1.0;

      Parameter newpar;
      newpar.name = parname;
      newpar.curvalue = parvalue;
      newpar.minvalue = minvalue;
      newpar.maxvalue = maxvalue;
      newpar.stepsize = stepsize;

      // If empty string, fit is default to be false
      bool fit = false;
      if (fitq.size() > 0)
      {
        if (fitq[0] == 'F' || fitq[0] == 'f')
          fit = true;
      }
      newpar.fit = fit;

      parammap.insert(make_pair(parname, newpar));
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Fit instrument parameters by non Monte Carlo algorithm
    * Requirement:  m_positionFunc should have the best fit result;
   */
  double RefinePowderInstrumentParameters3::execFitParametersNonMC()
  {
    // 1. Set up constraints
    setFunctionParameterFitSetups(m_positionFunc, m_profileParameters);

    // 2. Fit function
    // FIXME powerfit should be a user option before freezing this algorithm
    bool powerfit = false;
    double chi2 = fitFunction(m_positionFunc, m_dataWS, m_wsIndex, powerfit);

    // 2. Summary
    stringstream sumss;
    sumss << "Non-Monte Carlo Results:  Best Chi^2 = " << chi2;
    g_log.notice(sumss.str());

    return chi2;
  }

  //----------------------------------------------------------------------------------------------
  /** Refine instrument parameters by Monte Carlo/simulated annealing method
    */
  double RefinePowderInstrumentParameters3::execFitParametersMC()
  {
    // 1. Monte Carlo simulation
    double chisq = doSimulatedAnnealing(m_profileParameters);

    // 2. Summary
    stringstream sumss;
    sumss << "Monte Carlo Results:  Best Chi^2 = " << chisq << " @ Step "
          << m_bestChiSqStep << ", Group " << m_bestChiSqGroup;
    g_log.notice(sumss.str());

    return chisq;
  }

  //----------------------------------------------------------------------------------------------
  /** Do MC/simulated annealing to refine parameters
    *
    * Helpful:     double curchi2 = calculateD2TOFFunction(mFunction, domain, values, rawY, rawE);
    */
  double RefinePowderInstrumentParameters3::doSimulatedAnnealing(map<string, Parameter> inparammap)
  {
    // 1. Prepare/initialization
    //    Data structure
    const MantidVec& dataY = m_dataWS->readY(m_wsIndex);

    size_t numpts = dataY.size();

    vector<double> vecY(numpts, 0.0);

    //    Monte Carlo strategy and etc.
    vector<vector<string> > mcgroups;
    setupRandomWalkStrategy(inparammap, mcgroups);

    int randomseed = getProperty("MonteCarloRandomSeed");
    srand(randomseed);

    double temperature = getProperty("AnnealingTemperature");
    if (temperature < 1.0E-10)
      throw runtime_error("Annealing temperature is too low.");

    int maxiterations = getProperty("MonteCarloIterations");
    if (maxiterations <= 0)
      throw runtime_error("Max iteration cannot be 0 or less.");

    //    Book keeping
    map<string, Parameter> parammap;
    duplicateParameters(inparammap, parammap);
    // vector<pair<double, map<string, Parameter> > > bestresults;
    map<string, Parameter> bestresult;
    // size_t maxnumresults = 10;

    // 2. Set up parameters and get initial values
    m_bestChiSq = DBL_MAX;
    m_bestChiSqStep = -1;
    m_bestChiSqGroup = -1;

    double chisq0 = calculateFunction(parammap, vecY);
    double chisq0x = calculateFunctionError(m_positionFunc, m_dataWS, m_wsIndex);
    g_log.notice() << "[DBx510] Starting Chi^2 = " << chisq0 << " (homemade) "
                   << chisq0x << " (Levenber-marquadt)" << endl;

    bookKeepMCResult(parammap, chisq0, -1, -1, bestresult); // bestresults, maxnumresults);

    // 3. Monte Carlo starts
    double chisqx = chisq0;
    int numtotalacceptance = 0;
    int numrecentacceptance = 0;
    int numrecentsteps = 0;

    map<string, Parameter> propparammap; // parameters with proposed value
    duplicateParameters(parammap, propparammap);

    for (int istep = 0; istep < maxiterations; ++istep)
    {
      for (int igroup = 0; igroup < static_cast<int>(mcgroups.size()); ++igroup)
      {
        // a) Propose value
        proposeNewValues(mcgroups[igroup], parammap, propparammap, chisqx); // , prevbetterchi2);

        // b) Calcualte function and chi^2
        double propchisq = calculateFunction(propparammap, vecY);

        /*
        stringstream dbss;
        dbss << "[DBx541] New Chi^2 = " << propchisq << endl;
        vector<string> paramnames = m_positionFunc->getParameterNames();
        for (size_t i = 0; i < paramnames.size(); ++i)
        {
          string parname = paramnames[i];
          double curvalue = parammap[parname].value;
          double propvalue = propparammap[parname].value;
          dbss << parname << ":\t\t" << setw(20) << propvalue << "\t\t<-----\t\t" << curvalue << "\t Delta = "
               << curvalue-propvalue << endl;
        }
        g_log.notice(dbss.str());
        */

        // c) Determine to accept change
        bool acceptpropvalues = acceptOrDenyChange(propchisq, chisqx, temperature);

        // d) Change current chi^2, apply change, and book keep
        if (acceptpropvalues)
        {
          setFunctionParameterValues(m_positionFunc, propparammap);
          chisqx = propchisq;
          bookKeepMCResult(parammap, chisqx, istep, igroup, bestresult); // s, maxnumresults);
        }

        // e) MC strategy control
        ++ numtotalacceptance;
        ++ numrecentacceptance;
        ++ numrecentsteps;
      }

      // f) Annealing
      if (numrecentsteps >= 10)
      {
        double acceptratio = static_cast<double>(numrecentacceptance)/static_cast<double>(numrecentsteps);
        if (acceptratio < 0.2)
        {
          // i) Low acceptance, need to raise temperature
          temperature *= 2.0;
        }
        else if (acceptratio >= 0.8)
        {
          // ii) Temperature too high to accept too much new change
          temperature /= 2.0;
        }

        // iii) Reset counters
        numrecentacceptance = 0;
        numrecentsteps = 0;
      }
    }

    // 4. Apply the best result
    // sort(bestresults.begin(), bestresults.end());
    setFunctionParameterValues(m_positionFunc, bestresult);
    double chisqf = m_bestChiSq;

    g_log.warning() << "[DBx544] Best Chi^2 From MC = " << m_bestChiSq << endl;

    // 5. Use regular minimzer to try to get a better result
    string fitstatus;
    double fitchisq;
    bool goodfit = doFitFunction(m_positionFunc, m_dataWS, m_wsIndex, "Levenberg-MarquardtMD",
                                 1000, fitchisq, fitstatus);

    bool restoremcresult = false;
    if (goodfit)
    {
      map<string, Parameter> nullmap;
      fitchisq = calculateFunction(nullmap, vecY);
      if (fitchisq > chisqf)
      {
        // Fit is unable to achieve a better solution
        restoremcresult = true;
      }
      else
      {
        m_bestChiSq = fitchisq;
      }
    }
    else
    {
      // Fit is bad
      restoremcresult = true;
    }

    g_log.warning() << "[DBx545] Restore MC Result = " << restoremcresult << endl;

    if (restoremcresult)
    {
      setFunctionParameterValues(m_positionFunc, bestresult);
    }
    chisqf = m_bestChiSq;

    // 6. Final result
    double chisqfx = calculateFunctionError(m_positionFunc, m_dataWS, m_wsIndex);
    map<string, Parameter> emptymap;
    double chisqf0 = calculateFunction(emptymap, vecY);
    g_log.notice() << "Best Chi^2 (L-V) = " << chisqfx << ", (homemade) = " << chisqf0 << endl;
    g_log.warning() << "Data Size = " << m_dataWS->readX(m_wsIndex).size() << ", Number of parameters = "
                    << m_positionFunc->getParameterNames().size() << endl;

    return chisqf;
  }

  //----------------------------------------------------------------------------------------------
  /** Propose new parameters
    *
    * @param mcgroup:     list of parameters to have new values proposed
    * @param currchisq:  present chi^2 (as a factor in step size)
    * @param curparammap: current parameter maps
    * @param newparammap: parameters map containing new/proposed value
    */
  void RefinePowderInstrumentParameters3::proposeNewValues(vector<string> mcgroup,
                                                           map<string, Parameter>& curparammap,
                                                           map<string, Parameter>& newparammap,
                                                           double currchisq)
  {
    for (size_t i = 0; i < mcgroup.size(); ++i)
    {
      // random number between -1 and 1
      double randomnumber = 2*static_cast<double>(rand())/static_cast<double>(RAND_MAX) - 1.0;

      // parameter information
      string paramname = mcgroup[i];
      Parameter param = curparammap[paramname];
      double stepsize = m_dampingFactor * currchisq * (param.curvalue * param.mcA1 + param.mcA0) * randomnumber/m_bestChiSq;

      g_log.debug() << "Parameter " << paramname << " Step Size = " << stepsize
                    << " From " << param.mcA0 << ", " << param.mcA1 << ", " << param.curvalue
                    << ", " << m_dampingFactor << endl;

      // drunk walk or random walk
      double newvalue;
      // Random walk.  No preference on direction
      newvalue = param.curvalue + stepsize;

      /*
      if (m_walkStyle == RANDOMWALK)
      {

      }
      else if (m_walkStyle == DRUNKENWALK)
      {
        // Drunken walk.  Prefer to previous successful move direction
        int prevRightDirection;
        if (prevBetterRwp)
          prevRightDirection = 1;
        else
          prevRightDirection = -1;

        double randirint = static_cast<double>(rand())/static_cast<double>(RAND_MAX);

        // FIXME Here are some MAGIC numbers
        if (randirint < 0.1)
        {
          // Negative direction to previous direction
          stepsize = -1.0*fabs(stepsize)*static_cast<double>(param.movedirection*prevRightDirection);
        }
        else if (randirint < 0.4)
        {
          // No preferance
          stepsize = stepsize;
        }
        else
        {
          // Positive direction to previous direction
          stepsize = fabs(stepsize)*static_cast<double>(param.movedirection*prevRightDirection);
        }

        newvalue = param.value + stepsize;
      }
      else
      {
        newvalue = DBL_MAX;
        throw runtime_error("Unrecoganized walk style. ");
      }
      */

      // restriction
      if (param.nonnegative && newvalue < 0)
      {
        // If not allowed to be negative
        newvalue = fabs(newvalue);
      }

      // apply to new parameter map
      newparammap[paramname].curvalue = newvalue;

      // record some trace
      Parameter& p = curparammap[paramname];
      if (stepsize > 0)
      {
        p.movedirection = 1;
        ++ p.numpositivemove;
      }
      else if (stepsize < 0)
      {
        p.movedirection = -1;
        ++ p.numnegativemove;
      }
      else
      {
        p.movedirection = -1;
        ++p.numnomove;
      }
      p.sumstepsize += fabs(stepsize);
      if (fabs(stepsize) > p.maxabsstepsize)
        p.maxabsstepsize = fabs(stepsize);

      g_log.debug() << "[DBx257] " << paramname << "\t" << "Proposed value = " << setw(15)
                    << newvalue << " (orig = " << param.curvalue << ",  step = "
                    << stepsize << "), totRwp = " << currchisq << endl;
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Determine whether the proposed value should be accepted or denied
    *
    * @param curchisq:  present chi^2 (as a factor in step size)
    * @param newchisq:  new chi^2 (as a factor in step size)
    * @param temperature:  annealing temperature
    */
  bool RefinePowderInstrumentParameters3::acceptOrDenyChange(double curchisq, double newchisq,
                                                             double temperature)
  {
    bool accept;

    if (newchisq < curchisq)
    {
      // Lower Rwp.  Take the change
      accept = true;
    }
    else
    {
      // Higher Rwp. Take a chance to accept
      double dice = static_cast<double>(rand())/static_cast<double>(RAND_MAX);
      double bar = exp(-(newchisq-curchisq)/(curchisq*temperature));
      // double bar = exp(-(newrwp-currwp)/m_bestRwp);
      // g_log.notice() << "[DBx329] Bar = " << bar << ", Dice = " << dice << endl;
      if (dice < bar)
      {
        // random number (dice, 0 and 1) is smaller than bar (between -infty and 0)
        accept = true;
      }
      else
      {
        // Reject
        accept = false;
      }
    }

    return accept;
  }

  //----------------------------------------------------------------------------------------------
  /** Book keep the best fitting result
    */
  void RefinePowderInstrumentParameters3::bookKeepMCResult(map<string, Parameter> parammap,
                                                           double chisq, int istep, int igroup,
                                                           map<string, Parameter>& bestparammap)
                                                           // vector<pair<double, map<string, Parameter> > >& bestresults,
                                                           // size_t maxnumresults)
  {
    // 1. Check whether input Chi^2 is the best Chi^2
    bool recordparameter = false;

    if (chisq < m_bestChiSq)
    {
      m_bestChiSq = chisq;
      m_bestChiSqStep = istep;
      m_bestChiSqGroup = igroup;

      recordparameter = true;
    }

    // 2. Record for the best parameters
    if (bestparammap.size() == 0)
    {
      // No record yet
      duplicateParameters(parammap, bestparammap);
    }
    else if (recordparameter)
    {
      // Replace the record

    }

    // 2. Determine whether to add this entry to records
    /*
    bool addentry = true;
    if (bestresults.size() >= maxnumresults && chisq > bestresults.back().first)
      addentry = false;

    // 3. Add entry
    if (addentry)
    {
      map<string, Parameter> storemap;
      duplicateParameters(parammap, storemap);
      bestresults.push_back(make_pair(chisq, storemap));
      sort(bestresults.begin(), bestresults.end());
    }
    */

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Set up Monte Carlo random walk strategy
    */
  void RefinePowderInstrumentParameters3::setupRandomWalkStrategy(map<string, Parameter>& parammap,
                                                                  vector<vector<string> >& mcgroups)
  {
    stringstream dboutss;
    dboutss << "Monte Carlo minimizer refines: ";

    // 1. Monte Carlo groups
    // a. Instrument gemetry
    vector<string> geomparams;
    addParameterToMCMinimize(geomparams, "Dtt1", parammap);
    addParameterToMCMinimize(geomparams, "Dtt1t", parammap);
    addParameterToMCMinimize(geomparams, "Dtt2t", parammap);
    addParameterToMCMinimize(geomparams, "Zero", parammap);
    addParameterToMCMinimize(geomparams, "Zerot", parammap);
    addParameterToMCMinimize(geomparams, "Width", parammap);
    addParameterToMCMinimize(geomparams, "Tcross", parammap);
    mcgroups.push_back(geomparams);

    dboutss << "Geometry parameters: ";
    for (size_t i = 0; i < geomparams.size(); ++i)
      dboutss << geomparams[i] << "\t\t";
    dboutss << endl;

    g_log.notice(dboutss.str());

    // 2. Dictionary for each parameter for non-negative, mcX0, mcX1
    parammap["Width"].mcA0 = 0.0;
    parammap["Width"].mcA1 = 1.0;
    parammap["Width"].nonnegative = true;

    parammap["Tcross"].mcA0 = 0.0;
    parammap["Tcross"].mcA1 = 1.0;
    parammap["Tcross"].nonnegative = true;

    parammap["Zero"].mcA0 = 5.0;
    parammap["Zero"].mcA1 = 0.0;
    parammap["Zero"].nonnegative = false;

    parammap["Zerot"].mcA0 = 5.0;
    parammap["Zerot"].mcA1 = 0.0;
    parammap["Zerot"].nonnegative = false;

    parammap["Dtt1"].mcA0 = 5.0;
    parammap["Dtt1"].mcA1 = 0.0;
    parammap["Dtt1"].nonnegative = true;

    parammap["Dtt1t"].mcA0 = 5.0;
    parammap["Dtt1t"].mcA1 = 0.0;
    parammap["Dtt1t"].nonnegative = true;

    parammap["Dtt2t"].mcA0 = 0.1;
    parammap["Dtt2t"].mcA1 = 1.0;
    parammap["Dtt2t"].nonnegative = false;

    // 3. Reset
    map<string, Parameter>::iterator mapiter;
    for (mapiter = parammap.begin(); mapiter != parammap.end(); ++mapiter)
    {
      mapiter->second.movedirection = 1;
      mapiter->second.sumstepsize = 0.0;
      mapiter->second.numpositivemove = 0;
      mapiter->second.numnegativemove = 0;
      mapiter->second.numnomove = 0;
      mapiter->second.maxabsstepsize = -0.0;
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Add parameter (to a vector of string/name) for MC random walk
    * according to Fit in Parameter
    *
    * @param parnamesforMC: vector of parameter for MC minimizer
    * @param parname: name of parameter to check whether to put into refinement list
    * @param parammap :: parammap
    */
  void RefinePowderInstrumentParameters3::addParameterToMCMinimize(vector<string>& parnamesforMC,
                                                                   string parname,
                                                                   map<string, Parameter> parammap)
  {
    map<string, Parameter>::iterator pariter;
    pariter = parammap.find(parname);
    if (pariter == parammap.end())
    {
      stringstream errss;
      errss << "Parameter " << parname << " does not exisit Le Bail function parameters. ";
      g_log.error(errss.str());
      throw runtime_error(errss.str());
    }

    if (pariter->second.fit)
      parnamesforMC.push_back(parname);

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Implement parameter values, calculate function and its chi square.
    *
    * @param parammap:  if size = 0, there is no action to set function parameter.
    * @param vecY :: vecY
    * Return: chi^2
    */
  double RefinePowderInstrumentParameters3::calculateFunction(map<string, Parameter> parammap,
                                                              vector<double>& vecY)
  {
    // 1. Implement parameter values to m_positionFunc
    if (parammap.size() > 0)
      setFunctionParameterValues(m_positionFunc, parammap);

    // 2. Calculate
    const MantidVec& vecX = m_dataWS->readX(m_wsIndex);
    //    Check
    if (vecY.size() != vecX.size())
      throw runtime_error("vecY must be initialized with proper size!");

    m_positionFunc->function1D(vecY, vecX);

    // 3. Calcualte error
    double chisq = calculateFunctionChiSquare(vecY, m_dataWS->readY(m_wsIndex), m_dataWS->readE(m_wsIndex));

    return chisq;
  }

  //----------------------------------------------------------------------------------------------
  /** Calculate Chi^2
    */
  double calculateFunctionChiSquare(const vector<double> modelY, const vector<double> dataY,
                                    const vector<double> dataE)
  {
    // 1. Check
    if (modelY.size() != dataY.size() || dataY.size() != dataE.size())
      throw runtime_error("Input model, data and error have different size.");

    // 2. Calculation
    double chisq = 0.0;
    size_t numpts = modelY.size();
    for (size_t i = 0; i < numpts; ++i)
    {
      if (dataE[i] > 1.0E-5)
      {
        double temp = (modelY[i] - dataY[i])/dataE[i];
        chisq += temp*temp;
      }
    }

    return chisq;
  }

  //----------------------------------------------------------------------------------------------
  /** Calculate Chi^2 of the a function with all parameters are fixed
    */
  double RefinePowderInstrumentParameters3::calculateFunctionError(IFunction_sptr function,
                                                                   Workspace2D_sptr dataws, int wsindex)
  {
    // 1. Record the fitting information
    vector<string> parnames = function->getParameterNames();
    vector<bool> vecFix(parnames.size(), false);

    for (size_t i = 0; i < parnames.size(); ++i)
    {
      bool fixed = function->isFixed(i);
      vecFix[i] = fixed;
      if (!fixed)
        function->fix(i);
    }

    // 2. Fit with zero iteration
    double chi2;
    string fitstatus;
    doFitFunction(function, dataws, wsindex, "Levenberg-MarquardtMD", 0, chi2, fitstatus);

    // 3. Restore the fit/fix setup
    for (size_t i = 0; i < parnames.size(); ++i)
    {
      if (!vecFix[i])
        function->unfix(i);
    }

    return chi2;
  }

  //----------------------------------------------------------------------------------------------
  /** Fit a function by trying various minimizer or minimizer combination
    *
    * @param function :: an instance of a function to fit
    * @param dataws :: a workspace with the data
    * @param wsindex :: a histogram index
    * @param powerfit :: a flag to choose a robust algorithm to fit function
    *
    * Return: double chi2 of the final (best) solution.  If fitting fails, chi2 wil be maximum double
    */
  double RefinePowderInstrumentParameters3::fitFunction(IFunction_sptr function, Workspace2D_sptr dataws,
                                                        int wsindex, bool powerfit)
  {
    // 1. Store original
    map<string, pair<double, double> > start_paramvaluemap, paramvaluemap1, paramvaluemap2,
        paramvaluemap3;
    storeFunctionParameterValue(function, start_paramvaluemap);

    // 2. Calculate starting chi^2
    double startchisq = calculateFunctionError(function, dataws, wsindex);
    g_log.notice() << "[DBx436] Starting Chi^2 = " << startchisq << ", Power-Fit is " << powerfit << endl;

    // 3. Fitting
    int numiters;
    double final_chi2 = DBL_MAX;

    if (powerfit)
    {
      // a) Use Simplex to fit
      string minimizer = "Simplex";
      double chi2simplex;
      string fitstatussimplex;
      numiters = 10000;
      bool fitgood1 = doFitFunction(function, dataws, wsindex, minimizer, numiters,
                                    chi2simplex, fitstatussimplex);

      if (fitgood1)
        storeFunctionParameterValue(function, paramvaluemap1);
      else
        chi2simplex = DBL_MAX;

      // b) Continue Levenberg-Marquardt following Simplex
      minimizer = "Levenberg-MarquardtMD";
      double chi2lv2;
      string fitstatuslv2;
      numiters = 1000;
      bool fitgood2 = doFitFunction(function, dataws, wsindex, minimizer, numiters,
                                    chi2lv2, fitstatuslv2);
      if (fitgood2)
        storeFunctionParameterValue(function, paramvaluemap2);
      else
        chi2lv2 = DBL_MAX;

      // c) Fit by L.V. solely
      map<string, Parameter> tempparmap;
      restoreFunctionParameterValue(start_paramvaluemap, function, tempparmap);
      double chi2lv1;
      string fitstatuslv1;
      bool fitgood3 = doFitFunction(function, dataws, wsindex, minimizer, numiters,
                                    chi2lv1, fitstatuslv1);
      if (fitgood3)
        storeFunctionParameterValue(function, paramvaluemap3);
      else
        chi2lv1 = DBL_MAX;

      // 4. Compare best
      g_log.notice() << "Fit Result:  Chi2s: Simplex = " << chi2simplex << ", "
                     << "Levenberg 1 = " << chi2lv2 << ", Levenberg 2 = " << chi2lv1 << endl;

      if (fitgood1 || fitgood2 || fitgood3)
      {
        // At least one good fit
        if (fitgood1 && chi2simplex <= chi2lv2 && chi2simplex <= chi2lv1)
        {
          final_chi2 = chi2simplex;
          restoreFunctionParameterValue(paramvaluemap1, function, m_profileParameters);
        }
        else if (fitgood2 && chi2lv2 <= chi2lv1)
        {
          restoreFunctionParameterValue(paramvaluemap2, function, m_profileParameters);
          final_chi2 = chi2lv2;
        }
        else if (fitgood3)
        {
          final_chi2 = chi2lv1;
          restoreFunctionParameterValue(paramvaluemap3, function, m_profileParameters);
        }
        else
        {
          throw runtime_error("This situation is impossible to happen!");
        }
      } // END of Choosing Results
    }
    else
    {
      // 3B) Simple fit
      string minimizer = "Levenberg-MarquardtMD";
      string fitstatus;
      numiters = 1000;
      bool fitgood = doFitFunction(function, dataws, wsindex, minimizer, numiters,
                                   final_chi2, fitstatus);
      if (fitgood)
      {
        storeFunctionParameterValue(function, paramvaluemap1);
        restoreFunctionParameterValue(paramvaluemap1, function, m_profileParameters);
      }
      else
      {
        g_log.warning() << "Fit by " << minimizer << " failed.  Reason: " << fitstatus
                        << "\n";
      }
    }

    return final_chi2;
  }

  //----------------------------------------------------------------------------------------------
  /** Fit function
    * Minimizer: "Levenberg-MarquardtMD"/"Simplex"
   */
  bool RefinePowderInstrumentParameters3::doFitFunction(IFunction_sptr function, Workspace2D_sptr dataws, int wsindex,
                                                        string minimizer, int numiters, double& chi2, string& fitstatus)
  {
    // 0. Debug output
    stringstream outss;
    outss << "Fit function: " << m_positionFunc->asString() << endl << "Data To Fit: \n";
    for (size_t i = 0; i < dataws->readX(0).size(); ++i)
      outss << dataws->readX(wsindex)[i] << "\t\t" << dataws->readY(wsindex)[i] << "\t\t"
            << dataws->readE(wsindex)[i] << "\n";
    g_log.information() << outss.str();

    // 1. Create and setup fit algorithm
    API::IAlgorithm_sptr fitalg = createChildAlgorithm("Fit", 0.0, 0.2, true);
    fitalg->initialize();

    fitalg->setProperty("Function", function);
    fitalg->setProperty("InputWorkspace", dataws);
    fitalg->setProperty("WorkspaceIndex", wsindex);
    fitalg->setProperty("Minimizer", minimizer);
    fitalg->setProperty("CostFunction", "Least squares");
    fitalg->setProperty("MaxIterations", numiters);
    fitalg->setProperty("CalcErrors", true);

    // 2. Fit
    bool successfulfit = fitalg->execute();
    if (!fitalg->isExecuted() || ! successfulfit)
    {
      // Early return due to bad fit
      g_log.warning("Fitting to instrument geometry function failed. ");
      chi2 = DBL_MAX;
      fitstatus = "Minimizer throws exception.";
      return false;
    }

    // 3. Understand solution
    chi2 = fitalg->getProperty("OutputChi2overDoF");
    string tempfitstatus = fitalg->getProperty("OutputStatus");
    fitstatus = tempfitstatus;

    bool goodfit = fitstatus.compare("success") == 0;

    stringstream dbss;
    dbss << "Fit Result (GSL):  Chi^2 = " << chi2
         << "; Fit Status = " << fitstatus << ", Return Bool = " << goodfit << std::endl;
    vector<string> funcparnames = function->getParameterNames();
    for (size_t i = 0; i < funcparnames.size(); ++i)
      dbss << funcparnames[i] << " = " << setw(20) << function->getParameter(funcparnames[i])
           << " +/- " << function->getError(i) << "\n";
    g_log.debug() << dbss.str();

    return goodfit;
  }

  //----------------------------------------------------------------------------------------------
  /** Construct an output TableWorkspace for fitting result (profile parameters)
    */
  TableWorkspace_sptr RefinePowderInstrumentParameters3::genOutputProfileTable(map<string, Parameter> parameters,
                                                                     double startchi2, double finalchi2)
  {
    // 1. Create TableWorkspace
    TableWorkspace_sptr tablews(new TableWorkspace);

    tablews->addColumn("str", "Name");
    tablews->addColumn("double", "Value");
    tablews->addColumn("str", "FitOrTie");
    tablews->addColumn("double", "Min");
    tablews->addColumn("double", "Max");
    tablews->addColumn("double", "StepSize");
    tablews->addColumn("double", "Error");

    // 2. For chi^2
    addOrReplace(parameters, "Chi2_Init", startchi2);
    addOrReplace(parameters, "Chi2_Result", finalchi2);

    // 3. Set values
    map<string, Parameter>::iterator pariter;
    for (pariter = parameters.begin(); pariter != parameters.end(); ++pariter)
    {
      Parameter& param = pariter->second;
      TableRow newrow = tablews->appendRow();

      string fitortie;
      if (param.fit)
        fitortie = "fit";
      else
        fitortie = "tie";

      newrow << param.name << param.curvalue << fitortie << param.minvalue << param.maxvalue
             << param.stepsize << param.fiterror;
    }

    return tablews;
  }

  //----------------------------------------------------------------------------------------------
  /** Add a parameter to parameter map.  If this parametere does exist, then replace the value
    * of it.
    * @param parameters:  map
    * @param parname:     string, parameter name
    * @param parvalue:    double, parameter value
    */
  void RefinePowderInstrumentParameters3::addOrReplace(map<string, Parameter>& parameters,
                                                       string parname, double parvalue)
  {
    map<string, Parameter>::iterator pariter = parameters.find(parname);
    if (pariter != parameters.end())
    {
      parameters[parname].curvalue = parvalue;
    }
    else
    {
      Parameter newparameter;
      newparameter.name = parname;
      newparameter.curvalue = parvalue;
      parameters.insert(make_pair(parname, newparameter));
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Construct output
   */
  Workspace2D_sptr RefinePowderInstrumentParameters3::genOutputWorkspace(FunctionDomain1DVector domain,
                                                               FunctionValues rawvalues)
  {
    // 1. Create and set up output workspace
    size_t lenx = m_dataWS->readX(m_wsIndex).size();
    size_t leny = m_dataWS->readY(m_wsIndex).size();

    Workspace2D_sptr outws = boost::dynamic_pointer_cast<Workspace2D>
        (WorkspaceFactory::Instance().create("Workspace2D", 6, lenx, leny));

    outws->getAxis(0)->setUnit("dSpacing");

    TextAxis* taxis = new TextAxis(outws->getNumberHistograms());
    taxis->setLabel(0, "Data");
    taxis->setLabel(1, "Model");
    taxis->setLabel(2, "DiffDM");
    taxis->setLabel(3, "Start");
    taxis->setLabel(4, "DiffDS");
    taxis->setLabel(5, "Zdiff");
    outws->replaceAxis(1, taxis);

    // 3. Re-calculate values
    FunctionValues funcvalues(domain);
    m_positionFunc->function(domain, funcvalues);

    // 4. Add values
    // a) X axis
    for (size_t iws = 0; iws < outws->getNumberHistograms(); ++iws)
    {
      MantidVec& vecX = outws->dataX(iws);
      for (size_t n = 0; n < lenx; ++n)
        vecX[n] = domain[n];
    }

    // b) Y axis
    const MantidVec& dataY = m_dataWS->readY(m_wsIndex);

    for (size_t i = 0; i < domain.size(); ++i)
    {
      outws->dataY(0)[i] = dataY[i];
      outws->dataY(1)[i] = funcvalues[i];
      outws->dataY(2)[i] = dataY[i] - funcvalues[i];
      outws->dataY(3)[i] = rawvalues[i];
      outws->dataY(4)[i] = dataY[i] - rawvalues[i];
    }

    // 5. Zscore
    vector<double> zscore = Kernel::getZscore(outws->readY(2));
    for (size_t i = 0; i < domain.size(); ++i)
      outws->dataY(5)[i] = zscore[i];

    return outws;
  }

  //----------------------------------------------------------------------------------------------
  /** Set parameter values to function from Parameter map
   */
  void RefinePowderInstrumentParameters3::setFunctionParameterValues(IFunction_sptr function,
                                                                     map<string, Parameter> params)
  {
    // 1. Prepare
    vector<string> funparamnames = function->getParameterNames();

    // 2. Set up
    stringstream msgss;
    msgss << "Set Instrument Function Parameter : " << endl;

    std::map<std::string, Parameter>::iterator paramiter;
    for (size_t i = 0; i < funparamnames.size(); ++i)
    {
      string parname = funparamnames[i];
      paramiter = params.find(parname);

      if (paramiter != params.end())
      {
        // Found, set up the parameter
        Parameter& param = paramiter->second;
        function->setParameter(parname, param.curvalue);

        msgss << setw(10) << parname << " = " << param.curvalue << endl;
      }
      else
      {
        // Not found and thus quit
        stringstream errss;
        errss << "Peak profile parameter " << parname << " is not found in input parameters. ";
        g_log.error(errss.str());
        throw runtime_error(errss.str());
      }
    } // ENDFOR parameter name

    g_log.information(msgss.str());

    return;
  }

  /** Update parameter values to Parameter map from fuction map
  void RefinePowderInstrumentParameters3::updateFunctionParameterValues(IFunction_sptr function,
                                                              map<string, Parameter>& params)
  {
    // 1. Prepare
    vector<string> funparamnames = function->getParameterNames();

    // 2. Set up
    stringstream msgss;
    msgss << "Update Instrument Function Parameter To Storage Map : " << endl;

    std::map<std::string, Parameter>::iterator paramiter;
    for (size_t i = 0; i < funparamnames.size(); ++i)
    {
      string parname = funparamnames[i];
      paramiter = params.find(parname);

      if (paramiter != params.end())
      {
        // Found, set up the parameter
        Parameter& param = paramiter->second;
        param.prevalue = param.value;
        param.value = function->getParameter(parname);

        msgss << setw(10) << parname << " = " << param.value << endl;
      }
    } // ENDFOR parameter name

    g_log.information(msgss.str());

    return;
  }
  */

  //----------------------------------------------------------------------------------------------
  /** Set parameter fitting setup (boundary, fix or unfix) to function from Parameter map
   */
  void RefinePowderInstrumentParameters3::setFunctionParameterFitSetups(IFunction_sptr function,
                                                                        map<string, Parameter> params)
  {
    // 1. Prepare
    vector<string> funparamnames = m_positionFunc->getParameterNames();

    // 2. Set up
    std::map<std::string, Parameter>::iterator paramiter;
    for (size_t i = 0; i < funparamnames.size(); ++i)
    {
      string parname = funparamnames[i];
      paramiter = params.find(parname);

      if (paramiter != params.end())
      {
        // Found, set up the parameter
        Parameter& param = paramiter->second;
        if (param.fit)
        {
          // If fit.  Unfix it and set up constraint
          function->unfix(i);

          double lowerbound = param.minvalue;
          double upperbound = param.maxvalue;
          if (lowerbound >= -DBL_MAX*0.1 || upperbound <= DBL_MAX*0.1)
          {
            // If there is a boundary
            BoundaryConstraint *bc = new BoundaryConstraint(function.get(), parname, lowerbound,
                                                            upperbound, false);
            function->addConstraint(bc);
          }
        }
        else
        {
          // If fix.
          function->fix(i);
        }
      }
      else
      {
        // Not found and thus quit
        stringstream errss;
        errss << "Peak profile parameter " << parname << " is not found in input parameters. ";
        g_log.error(errss.str());
        throw runtime_error(errss.str());
      }
    } // ENDFOR parameter name

    g_log.notice() << "Fit function:\n" << function->asString() << "\n";

    return;
  }


  //================================= External Functions =========================================

  //----------------------------------------------------------------------------------------------
  /** Copy parameters from source to target, i.e., clear the target and make it exacly same as
    * source;
    */
  void duplicateParameters(map<string, Parameter> source, map<string, Parameter>& target)
  {
    target.clear();

    map<string, Parameter>::iterator miter;
    for (miter = source.begin(); miter != source.end(); ++miter)
    {
      string parname = miter->first;
      Parameter param = miter->second;
      Parameter newparam;
      newparam = param;
      target.insert(make_pair(parname, newparam));
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Copy parameters from source to target, i.e., clear the target and make it exacly same as
    * source;
    */
  void copyParametersValues(map<string, Parameter> source, map<string, Parameter>& target)
  {
    // 1. Check
    if (source.size() != target.size())
      throw runtime_error("Source and Target should have the same size.");

    // 2. Copy the value
    map<string, Parameter>::iterator miter;
    map<string, Parameter>::iterator titer;
    for (miter = source.begin(); miter != source.end(); ++miter)
    {
      string parname = miter->first;
      Parameter param = miter->second;
      double paramvalue = param.curvalue;

      titer = target.find(parname);
      if (titer == target.end())
        throw runtime_error("Source and target should have exactly the same keys.");

      titer->second.curvalue = paramvalue;
    }

    return;
  }


  //----------------------------------------------------------------------------------------------
  /** Convert a vector to a lookup map (dictionary)
   */
  void convertToDict(vector<string> strvec, map<string, size_t>& lookupdict)
  {
    lookupdict.clear();

    for (size_t i = 0; i < strvec.size(); ++i)
      lookupdict.insert(make_pair(strvec[i], i));

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Get the index from lookup dictionary (map)
   */
  int getStringIndex(map<string, size_t> lookupdict, string key)
  {
    map<string, size_t>::iterator fiter;
    fiter = lookupdict.find(key);

    int returnvalue;

    if (fiter == lookupdict.end())
    {
      // does not exist
      returnvalue = -1;
    }
    else
    {
      // exist
      returnvalue = static_cast<int>(fiter->second);
    }

    return returnvalue;
  }

  //----------------------------------------------------------------------------------------------
  /** Store function parameter values to a map
    */
  void storeFunctionParameterValue(IFunction_sptr function, map<string, pair<double, double> >& parvaluemap)
  {
    parvaluemap.clear();

    vector<string> parnames = function->getParameterNames();
    for (size_t i = 0; i < parnames.size(); ++i)
    {
      string& parname = parnames[i];
      double parvalue = function->getParameter(i);
      double parerror = function->getError(i);
      parvaluemap.insert(make_pair(parname, make_pair(parvalue, parerror)));
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Restore function parameter values saved in a (string,double) map to a function object
    * and a (string, Parameter) map
    */
  void restoreFunctionParameterValue(map<string, pair<double, double> > parvaluemap, IFunction_sptr function,
                                     map<string, Parameter>& parammap)
  {
    vector<string> parnames = function->getParameterNames();

    for (size_t i = 0; i < parnames.size(); ++i)
    {
      string& parname = parnames[i];
      map<string, pair<double, double> >::iterator miter;
      miter = parvaluemap.find(parname);

      if (miter != parvaluemap.end())
      {
        double parvalue = miter->second.first;
        double parerror = miter->second.second;

        // 1. Function
        function->setParameter(parname, parvalue);

        // 2. Parameter map
        map<string, Parameter>::iterator pariter = parammap.find(parname);
        if (pariter != parammap.end())
        {
          // Find the entry
          pariter->second.curvalue = parvalue;
          pariter->second.fiterror = parerror;
        }
      }
    }

    return;
  }

} // namespace CurveFitting
} // namespace Mantid