#ifndef TEMPUS_TIMESTEPCONTROL_IMPL_HPP
#define TEMPUS_TIMESTEPCONTROL_IMPL_HPP

#include "Tempus_TimeStepControl.hpp"

namespace {

  static std::string timeMin_name    = "Minimum simulation time";
  static Scalar      timeMin_default = 0.0;

  static std::string timeMax_name    = "Maximum simulation time";
  static Scalar      timeMax_default = 100.0;

  static std::string dtMin_name    = "Minimum time step";
  static Scalar      dtMin_default = 0.0;

  static std::string dtMax_name    = "Maximum time step";
  static Scalar      dtMax_default = 100.0;

  static std::string iStepMin_name    = "Minimum time step index";
  static int         iStepMin_default = 0;

  static std::string iStepMax_name    = "Maximum time step index";
  static int         iStepMax_default = 100;

  static std::string errorMaxAbs_name    = "Maximum absolute error";
  static Scalar      errorMaxAbs_default = 1.0e-08;

  static std::string errorMaxRel_name    = "Maximum relative error";
  static Scalar      errorMaxRel_default = 1.0;

  static std::string orderMin_name    = "Minimum time integration order";
  static int         orderMin_default = 1;

  static std::string orderMax_name    = "Maximum time integration order";
  static int         orderMax_default = 4;

  static std::string Constant_name     = "Constant";
  static std::string Variable_name     = "Variable";
  static std::string Selection_name    = "Step Type";
  static std::string Selection_default = Variable_name;

  Teuchos::Array<std::string> StepType_names = Teuchos::tuple<std::string>(
      Constant_name,
      Variable_name);

  const Teuchos::RCP<Teuchos::StringToIntegralParameterEntryValidator<tempus::StepType> >
    StepTypeValidator = Teuchos::rcp(
        new Teuchos::StringToIntegralParameterEntryValidator<tempus::StepType>(
          StepType_names,
          Teuchos::tuple<tempus::StepType>(
            tempus::CONSTANT_STEP_SIZE,
            tempus::VARIABLE_STEP_SIZE),
          Selection_name));

  static std::string dtConstant_name             = "Constant Time Step Size";
  static Scalar      dtConstant_default          = 1.0;
  static std::string outputTimeList_name         = "Output Time List";
  static std::string outputTimeList_default      = "";
  static std::string outputIndexList_name        = "Output Index List";
  static std::string outputIndexList_default     = "";
  static std::string outputTimeInterval_name     = "Output Time Interval";
  static Scalar      outputTimeInterval_default  = 100.0;
  static std::string outputIndexInterval_name    = "Output Index Interval";
  static int         outputIndexInterval_default = 100;

} // namespace


namespace tempus {

// TimeStepControl definitions:
template<class Scalar>
TimeStepControl<Scalar>::TimeStepControl()
{
  paramList = getValidParameters();
  this->setParameterList(paramList);
}

template<class Scalar>
TimeStepControl<Scalar>::TimeStepControl(
  RCP<Teuchos::ParameterList> paramList_ = Teuchos::null )
{
  if ( paramList_ == Teuchos::null )
    paramList = getValidParameters();
  else
    paramList = paramList_;

  this->setParameterList(paramList);
}

template<class Scalar>
TimeStepControl<Scalar>::TimeStepControl( const TimeStepControl<Scalar>& tsc_ )
  : timeMin      (tsc_.timeMin    ),
    timeMax      (tsc_.timeMax    ),
    dtMin        (tsc_.dtMin      ),
    dtMax        (tsc_.dtMax      ),
    iStepMin     (tsc_.iStepMin   ),
    iStepMax     (tsc_.iStepMax   ),
    errorMaxAbs  (tsc_.errorMaxAbs),
    errorMaxRel  (tsc_.errorMaxRel),
    orderMin     (tsc_.orderMin   ),
    orderMax     (tsc_.orderMax   ),
    stepType     (tsc_.stepType   ),
    dtConstant   (tsc_.dtConstant ),
    outputIndices(tsc_.outputIndices),
    outputTimes  (tsc_.outputTimes),
    paramList    (tsc_.paramList  )
{}



template<class Scalar>
void TimeStepControl::setNextTimeStep(const Scalar time,
  const int iStep, const Scalar errorAbs, const Scalar errorRel,
  const int order, Scalar dt, bool output) const
{
  output = false;

  TEUCHOS_TEST_FOR_EXCEPTION(
    ( dt <= ST::zero() ), std::out_of_range,
    "Error - Time step size is less than or equal zero.\n"
    "        dt = " << dt << "\n");

  TEUCHOS_TEST_FOR_EXCEPTION(
    (iStep + 1 < iStepMin || iStep + 1 > iStepMax ), std::out_of_range,
    "Error - Time step index is out of range.\n"
    << "    [iStepMin, iStepMax] = [" << iStepMin << ", " << iStepMax << "]\n"
    << "    iStep + 1 = " << iStep << "\n");

  if ( outputIndices.find(iStep) ) output = true;

  if (stepType == CONSTANT_STEP_SIZE) {

    TEUCHOS_TEST_FOR_EXCEPTION(
      ( dt != dtConstant ), std::out_of_range,
      "Error - ( dt = "<< dt <<") != ( dtConstant = "<< dtConstant <<" )!\n");

    if (time + dt < timeMin || time + dt > timeMax ) {
      RCP<Teuchos::FancyOStream> out = this->getOStream();
      Teuchos::OSTab ostab(out,1,"setNextTimeStep");
      *out << "Warning - Time step 'steps' outside desired time range.\n"
           << "    [t_min, t_max] = [" << timeMin << ", " << timeMax << "]\n"
           << "    T + dt = "<< time <<" + "<< dt<<" = "<< time + dt <<"\n";
    }

    TEUCHOS_TEST_FOR_EXCEPTION(
      (errorAbs > errorAbsMax ), std::out_of_range,
      "Error - Solution absolute error is too large and can not change "
      "time step size!\n"
      "    Time step type == CONSTANT_STEP_SIZE\n"
      "    (errorAbs = "<<errorAbs<<") > (errorMaxAbs = "<<errorAbsMax<<")\n");

    TEUCHOS_TEST_FOR_EXCEPTION(
      (errorRel > errorRelMax ), std::out_of_range,
      "Error - Solution relative error is too large and can not change "
      "time step size!\n"
      "    Time step type == CONSTANT_STEP_SIZE\n"
      "    (errorRel = "<<errorRel<<") > (errorMaxRel = "<<errorRelMax<<")\n");

    TEUCHOS_TEST_FOR_EXCEPTION(
      (order < orderMin || order > orderMax ), std::out_of_range,
      "Error - Solution order is out of range and can not change "
      "time step size!\n"
      "    Time step type == CONSTANT_STEP_SIZE\n"
      "    [order_min, order_max] = [" << orderMin << ", " << orderMax << "]\n"
      "    order = " << order << "\n");

    if (!output) {
      for (int i=0; i < outputTimes.size(); ++i) {
        if (time < outputTimes[i] && outputTimes[i] <= time + dt) {
          output = true;
          break;
        }
      }
    }

  } else { // VARIABLE_STEP_SIZE

    TEUCHOS_TEST_FOR_EXCEPTION(
      (time + dt < timeMin ), std::out_of_range,
      "Error - Time step does not 'step' INTO time range.\n"
      "    [t_min, t_max] = [" << timeMin << ", " << timeMax << "]\n"
      "    T + dt = " << time <<" + "<< dt <<" = " << time + dt << "\n");

    // \todo The following controls should be generalized to plugable options.
    if (errorAbs > errorMaxAbs) dt /= 2;
    if (errorRel > errorMaxRel) dt /= 2;
    if (order < orderMin) dt *= 2;
    if (order > orderMax) dt /= 2;

    if (time + dt > timeMax ) dt = timeMax - time;

    if (!output) {
      for (int i=0; i < outputTimes.size(); ++i) {
        if (time < outputTimes[i] && outputTimes[i] <= time + dt) {
          output = true;
          dt = outputTimes[i] - time;
          break;
        }
      }
    }
  }

}


template<class Scalar>
std::string TimeStepControl<Scalar>::description() const
{
  std::string name = "Tempus::TimeStepControl";
  return(name);
}


template<class Scalar>
void TimeStepControl<Scalar>::describe(
   Teuchos::FancyOStream               &out,
   const Teuchos::EVerbosityLevel      verbLevel) const
{
  if (verbLevel == Teuchos::VERB_EXTREME) {
    out << description() << "::describe:" << std::endl
        << "timeMin     = " << timeMin     << std::endl
        << "timeMax     = " << timeMax     << std::endl
        << "dtMin       = " << dtMin       << std::endl
        << "dtMax       = " << dtMax       << std::endl
        << "iStepMin    = " << iStepMin    << std::endl
        << "iStepMax    = " << iStepMax    << std::endl
        << "errorMaxAbs = " << errorMaxAbs << std::endl
        << "errorMaxRel = " << errorMaxRel << std::endl
        << "orderMin    = " << orderMin    << std::endl
        << "orderMax    = " << orderMax    << std::endl
        << "stepType    = " << stepType    << std::endl
        << "paramList   = " << paramList   << std::endl;
}


template <class Scalar>
void TimeStepControl<Scalar>::setParameterList(
  RCP<Teuchos::ParameterList> const& paramList_)
{
  TEUCHOS_TEST_FOR_EXCEPT( is_null(paramList_) );
  paramList_->validateParameters(*this->getValidParameters());
  paramList = paramList_;

  Teuchos::readVerboseObjectSublist(&*paramList,this);

  timeMin     = paramList->get<double>(timeMin_name    , timeMin_default    );
  timeMax     = paramList->get<double>(timeMax_name    , timeMax_default    );
  TEUCHOS_TEST_FOR_EXCEPTION(
    (timeMin > timeMax ), std::logic_error,
    "Error - Inconsistent time range.\n"
    "    (timeMin = "<<timeMin<<") > (timeMax = "<<timeMax<<")\n");

  dtMin       = paramList->get<double>(dtMin_name      , dtMin_default      );
  dtMax       = paramList->get<double>(dtMax_name      , dtMax_default      );
  TEUCHOS_TEST_FOR_EXCEPTION(
    (dtMin < ST::zero() ), std::logic_error,
    "Error - Negative minimum time step.  dtMin = "<<dtMin<<")\n");
  TEUCHOS_TEST_FOR_EXCEPTION(
    (dtMax < ST::zero() ), std::logic_error,
    "Error - Negative maximum time step.  dtMax = "<<dtMax<<")\n");
  TEUCHOS_TEST_FOR_EXCEPTION(
    (dtMin > dtMax ), std::logic_error,
    "Error - Inconsistent time step range.\n"
    "    (dtMin = "<<dtMin<<") > (dtMax = "<<dtMax<<")\n");

  iStepMin    = paramList->get<int>   (iStepMin_name   , iStepMin_default   );
  iStepMax    = paramList->get<int>   (iStepMax_name   , iStepMax_default   );
  TEUCHOS_TEST_FOR_EXCEPTION(
    (iStepMin > iStepMax ), std::logic_error,
    "Error - Inconsistent time index range.\n"
    "    (iStepMin = "<<iStepMin<<") > (iStepMax = "<<iStepMax<<")\n");

  errorMaxAbs = paramList->get<double>(errorMaxAbs_name, errorMaxAbs_default);
  errorMaxRel = paramList->get<double>(errorMaxRel_name, errorMaxRel_default);
  TEUCHOS_TEST_FOR_EXCEPTION(
    (errorMaxAbs < ST::zero() ), std::logic_error,
    "Error - Negative maximum time step.  errorMaxAbs = "<<errorMaxAbs<<")\n");
  TEUCHOS_TEST_FOR_EXCEPTION(
    (errorMaxRel < ST::zero() ), std::logic_error,
    "Error - Negative maximum time step.  errorMaxRel = "<<errorMaxRel<<")\n");

  orderMin    = paramList->get<int>   (orderMin_name   , orderMin_default   );
  orderMax    = paramList->get<int>   (orderMax_name   , orderMax_default   );
  TEUCHOS_TEST_FOR_EXCEPTION(
    (orderMin < ST::zero() ), std::logic_error,
    "Error - Negative minimum order.  orderMin = "<<orderMin<<")\n");
  TEUCHOS_TEST_FOR_EXCEPTION(
    (orderMax < ST::zero() ), std::logic_error,
    "Error - Negative maximum order.  orderMax = "<<orderMax<<")\n");
  TEUCHOS_TEST_FOR_EXCEPTION(
    (orderMin > orderMax ), std::logic_error,
    "Error - Inconsistent order range.\n"
    "    (orderMin = "<<orderMin<<") > (orderMax = "<<orderMax<<")\n");

  stepType = StepTypePolicyValidator->getIntegralValue(
      *paramList, Selection_name, Selection_default);

  dtConstant  = paramList->get<double>(dtConstant_name , dtConstant_default);
  TEUCHOS_TEST_FOR_EXCEPTION(
    (dtConstant < ST::zero() ), std::logic_error,
    "Error - Negative constant time step.  dtConstant = "<<dtConstant<<")\n");
  TEUCHOS_TEST_FOR_EXCEPTION(
    (dtConstant < dtMin || dtConstant > dtMax ), std::out_of_range,
    "Error - Constant time step is out of range.\n"
    << "    [dtMin, dtMax] = [" << dtMin << ", " << dtMax << "]\n"
    << "    dtConstant = " << dtConstant << "\n");


  // Parse output times
  {
    outputTimes.clear();
    std::string str =
      paramList->get<std::string>(outputTimeList_name,outputTimeList_default);
    std::string delimiters(",");
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
    while ( (pos != std::string::npos) || (lastPos != std::string::npos) ) {
      std::string token = str.substr(lastPos,pos-lastPos);
      outputTimes.push_back(Scalar(std::stod(token)));
      if(pos==std::string::npos)
        break;

      lastPos = str.find_first_not_of(delimiters, pos);
      pos = str.find_first_of(delimiters, lastPos);
    }

    Scalar outputTimeInterval =
     paramList->get<double>(outputTimeInterval_name,outputTimeInterval_default);
    Scalar output_t = timeMin;
    while ( output_t <= timeMax ) {
      outputTimes.push_back(output_t);
      output_t += outputTimeInterval;
    }

    // order output times
    std::sort(outputTimes.begin(),outputTimes.end());
  }

  // Parse output indices
  {
    outputIndices.clear();
    std::string str =
      paramList->get<std::string>(outputIndexList_name,outputIndexList_default);
    std::string delimiters(",");
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
    while ( (pos != std::string::npos) || (lastPos != std::string::npos) ) {
      std::string token = str.substr(lastPos,pos-lastPos);
      outputIndices.push_back(int(std::stoi(token)));
      if(pos==std::string::npos)
        break;

      lastPos = str.find_first_not_of(delimiters, pos);
      pos = str.find_first_of(delimiters, lastPos);
    }

    Scalar outputIndexInterval =
      paramList->get<int>(outputIndexInterval_name,outputIndexInterval_default);
    Scalar output_i = iStepMin;
    while ( output_i <= iStepMax ) {
      outputIndices.push_back(output_i);
      output_i += outputIndexInterval;
    }

    // order output indices
    std::sort(outputIndices.begin(),outputIndices.end());
  }

  Teuchos::readVerboseObjectSublist(&*paramList,this);
}


template<class Scalar>
RCP<const Teuchos::ParameterList> TimeStepControl<Scalar>::getValidParameters() const
{
  static RCP<Teuchos::ParameterList> validPL;

  if (is_null(validPL)) {

    RCP<Teuchos::ParameterList> pl = Teuchos::parameterList();
    Teuchos::setupVerboseObjectSublist(&*pl);

    pl->set(timeMin_name    , timeMin_default    , "Minimum simulation time");
    pl->set(timeMax_name    , timeMax_default    , "Maximum simulation time");
    pl->set(dtMin_name      , dtMin_default      , "Minimum time step size");
    pl->set(dtMax_name      , dtMax_default      , "Maximum time step size");
    pl->set(iStepMin_name   , iStepMin_default   , "Minimum time step index");
    pl->set(iStepMax_name   , iStepMax_default   , "Maximum time step index");
    pl->set(errorMaxAbs_name, errorMaxAbs_default, "Maximum absolute error");
    pl->set(errorMaxRel_name, errorMaxRel_default, "Maximum relative error");
    pl->set(orderMin_name, orderMin_default, "Minimum time integration order");
    pl->set(orderMax_name, orderMax_default, "Maximum time integration order");

    pl->set(Selection_name, Selection_default,
      "Step Type indicates whether the Integrator will allow the time step "
      "to be modified the Stepper.\n"
      "  'Constant' - Integrator will take constant time step sizes.\n"
      "  'Variable' - Integrator will allow changes to the time step size.\n"
//      "'Unmodifiable' - Integrator will not allow the Stepper to take a "
//                     "time step different than one requested.\n"
//      "'Modifiable' - Integrator will use a time step size from the Stepper "
//                     "that is different than the one requested.\n"
      StepTypeValidator);

    pl->set(outputTimeList_name, outputTimeList_default,
      "Comma deliminated list of output times");
    pl->set(outputIndexList_name, outputIndexList_default,
      "Comma deliminated list of output indices");
    pl->set(outputTimeInterval_name, outputTimeInterval_default,
      "Output time interval (e.g., every 100.0 integrated time");
    pl->set(outputIndexInterval_name, outputIndexInterval_default,
      "Output index interval (e.g., every 100 time steps");

    validPL = pl;

  }
  return validPL;
}


template <class Scalar>
RCP<Teuchos::ParameterList>
TimeStepControl<Scalar>::getNonconstParameterList()
{
  return(paramList);
}


template <class Scalar>
RCP<Teuchos::ParameterList> TimeStepControl<Scalar>::unsetParameterList()
{
  RCP<Teuchos::ParameterList> temp_param_list = paramList;
  paramList = Teuchos::null;
  return(temp_param_list);
}


} // namespace tempus
#endif TEMPUS_TIMESTEPCONTROL_IMPL_HPP
