set ( SRC_FILES
	src/Algorithm.cpp
	src/AlgorithmFactory.cpp
	src/AlgorithmHasProperty.cpp
	src/AlgorithmHistory.cpp
	src/AlgorithmManager.cpp
	src/AlgorithmObserver.cpp
	src/AlgorithmProperty.cpp
	src/AlgorithmProxy.cpp
	src/AnalysisDataService.cpp
	src/ArchiveSearchFactory.cpp
	src/Axis.cpp
	src/BoxController.cpp
	src/CatalogFactory.cpp
	src/CatalogManager.cpp
	src/CatalogSession.cpp
	src/ChopperModel.cpp
	src/Column.cpp
	src/ColumnFactory.cpp
	src/CompositeCatalog.cpp
	src/CompositeDomainMD.cpp
	src/CompositeFunction.cpp
	src/ConstraintFactory.cpp
	src/CoordTransform.cpp
	src/CostFunctionFactory.cpp
	src/DataProcessorAlgorithm.cpp
	src/DeprecatedAlgorithm.cpp
	src/DomainCreatorFactory.cpp
	src/EnabledWhenWorkspaceIsType.cpp
	src/ExperimentInfo.cpp
	src/Expression.cpp
	src/FermiChopperModel.cpp
	src/FileFinder.cpp
	src/FileLoaderRegistry.cpp
	src/FileProperty.cpp
	src/FrameworkManager.cpp
	src/FuncMinimizerFactory.cpp
	src/FunctionDomain1D.cpp
	src/FunctionDomainMD.cpp
	src/FunctionFactory.cpp
	src/FunctionProperty.cpp
	src/FunctionValues.cpp
	src/GridDomain.cpp
	src/GridDomain1D.cpp
	src/IDomainCreator.cpp
	src/IEventList.cpp
	src/IEventWorkspace.cpp
	src/IFuncMinimizer.cpp
	src/IFunction.cpp
	src/IFunction1D.cpp
	src/IFunctionMD.cpp
	src/IFunctionMW.cpp
	src/ILiveListener.cpp
	src/IMDEventWorkspace.cpp
	src/IMDHistoWorkspace.cpp
	src/IMDIterator.cpp
	src/IMDWorkspace.cpp
	src/IPeak.cpp
	src/IPeakFunction.cpp
	src/IPeaksWorkspace.cpp
	src/IPowderDiffPeakFunction.cpp
	src/ISpectrum.cpp
	src/ISplittersWorkspace.cpp
	src/ITableWorkspace.cpp
	src/IkedaCarpenterModerator.cpp
	src/ImmutableCompositeFunction.cpp
	src/ImplicitFunctionParameterParserFactory.cpp
	src/ImplicitFunctionParserFactory.cpp
	src/ImplicitFunctionFactory.cpp
	src/InstrumentDataService.cpp
	src/JointDomain.cpp
	src/LinearScale.cpp
	src/LiveListenerFactory.cpp
	src/LogManager.cpp
	src/LogarithmScale.cpp
	src/MDGeometry.cpp
	src/MatrixWSIndexCalculator.cpp
	src/MatrixWorkspace.cpp
	src/MatrixWorkspaceMDIterator.cpp
	src/MemoryManager.cpp
	src/ModeratorModel.cpp
	src/MultiDomainFunction.cpp
	src/MultiPeriodGroupAlgorithm.cpp
	src/MultipleExperimentInfos.cpp
	src/MultipleFileProperty.cpp
	src/NullCoordTransform.cpp
	src/NumericAxis.cpp
	src/ParamFunction.cpp
	src/ParameterReference.cpp
	src/ParameterTie.cpp
	src/Progress.cpp
	src/PropertyManagerDataService.cpp
	src/PropertyNexus.cpp
	src/RefAxis.cpp
	src/Run.cpp
	src/Sample.cpp
	src/SampleEnvironment.cpp
	src/ScopedWorkspace.cpp
	src/ScriptRepository.cpp
	src/ScriptRepositoryFactory.cpp
	src/SpectraAxis.cpp
	src/SpectrumDetectorMapping.cpp
	src/TableRow.cpp
	src/TextAxis.cpp
	src/TransformScaleFactory.cpp
	src/Workspace.cpp
	src/WorkspaceFactory.cpp
	src/WorkspaceGroup.cpp
	src/WorkspaceHistory.cpp
	src/WorkspaceOpOverloads.cpp
	src/WorkspaceProperty.cpp
)

set ( SRC_UNITY_IGNORE_FILES src/CompositeFunction.cpp
        src/IDataFileChecker.cpp
        src/MatrixWorkspace.cpp
        src/IEventWorkspace.cpp
        src/IFunctionMD.cpp
        src/IPeakFunction.cpp
	src/IPowderDiffPeakFunction.cpp
        )

set ( INC_FILES
	#	inc/MantidAPI/BoxCtrlChangesInterface.h
	inc/MantidAPI/Algorithm.h
	inc/MantidAPI/AlgorithmFactory.h
	inc/MantidAPI/AlgorithmHasProperty.h
	inc/MantidAPI/AlgorithmHistory.h
	inc/MantidAPI/AlgorithmManager.h
	inc/MantidAPI/AlgorithmObserver.h
	inc/MantidAPI/AlgorithmProperty.h
	inc/MantidAPI/AlgorithmProxy.h
	inc/MantidAPI/AnalysisDataService.h
	inc/MantidAPI/ArchiveSearchFactory.h
	inc/MantidAPI/Axis.h
	inc/MantidAPI/BoxController.h
	inc/MantidAPI/CatalogFactory.h
	inc/MantidAPI/CatalogManager.h
	inc/MantidAPI/CatalogSession.h
	inc/MantidAPI/ChopperModel.h
	inc/MantidAPI/Column.h
	inc/MantidAPI/ColumnFactory.h
	inc/MantidAPI/CompositeCatalog.h
	inc/MantidAPI/CompositeDomain.h
	inc/MantidAPI/CompositeDomainMD.h
	inc/MantidAPI/CompositeFunction.h
	inc/MantidAPI/ConstraintFactory.h
	inc/MantidAPI/CoordTransform.h
	inc/MantidAPI/CostFunctionFactory.h
	inc/MantidAPI/DataProcessorAlgorithm.h
	inc/MantidAPI/DeclareUserAlg.h
	inc/MantidAPI/DeprecatedAlgorithm.h
	inc/MantidAPI/DllConfig.h
	inc/MantidAPI/DomainCreatorFactory.h
	inc/MantidAPI/EnabledWhenWorkspaceIsType.h
	inc/MantidAPI/ExperimentInfo.h
	inc/MantidAPI/Expression.h
	inc/MantidAPI/FermiChopperModel.h
	inc/MantidAPI/FileFinder.h
	inc/MantidAPI/FileLoaderRegistry.h
	inc/MantidAPI/FileProperty.h
	inc/MantidAPI/FrameworkManager.h
	inc/MantidAPI/FuncMinimizerFactory.h
	inc/MantidAPI/FunctionDomain.h
	inc/MantidAPI/FunctionDomain1D.h
	inc/MantidAPI/FunctionDomainMD.h
	inc/MantidAPI/FunctionFactory.h
	inc/MantidAPI/FunctionProperty.h
	inc/MantidAPI/FunctionValues.h
	inc/MantidAPI/GridDomain.h
	inc/MantidAPI/GridDomain1D.h
	inc/MantidAPI/IAlgorithm.h
	inc/MantidAPI/IArchiveSearch.h
	inc/MantidAPI/IBackgroundFunction.h
	inc/MantidAPI/IBoxControllerIO.h
	inc/MantidAPI/ICatalog.h
	inc/MantidAPI/ICatalogInfoService.h
	inc/MantidAPI/IConstraint.h
	inc/MantidAPI/ICostFunction.h
	inc/MantidAPI/IDomainCreator.h
	inc/MantidAPI/IEventList.h
	inc/MantidAPI/IEventWorkspace.h
	inc/MantidAPI/IFileLoader.h
	inc/MantidAPI/IFuncMinimizer.h
	inc/MantidAPI/IFunction.h
	inc/MantidAPI/IFunction1D.h
	inc/MantidAPI/IFunctionMD.h
	inc/MantidAPI/IFunctionMW.h
	inc/MantidAPI/IFunctionValues.h
	inc/MantidAPI/IFunctionWithLocation.h
	inc/MantidAPI/ILiveListener.h
	inc/MantidAPI/IMDEventWorkspace.h
	inc/MantidAPI/IMDHistoWorkspace.h
	inc/MantidAPI/IMDIterator.h
	inc/MantidAPI/IMDNode.h
	inc/MantidAPI/IMDWorkspace.h
	inc/MantidAPI/IMaskWorkspace.h
	inc/MantidAPI/IPeak.h
	inc/MantidAPI/IPeakFunction.h
	inc/MantidAPI/IPeaksWorkspace.h
	inc/MantidAPI/IPowderDiffPeakFunction.h
	inc/MantidAPI/ISpectrum.h
	inc/MantidAPI/ISplittersWorkspace.h
	inc/MantidAPI/ITableWorkspace.h
	inc/MantidAPI/ITransformScale.h
	inc/MantidAPI/IWorkspaceProperty.h
	inc/MantidAPI/IkedaCarpenterModerator.h
	inc/MantidAPI/ImmutableCompositeFunction.h
	inc/MantidAPI/ImplicitFunctionBuilder.h
	inc/MantidAPI/ImplicitFunctionFactory.h
	inc/MantidAPI/ImplicitFunctionParameter.h
	inc/MantidAPI/ImplicitFunctionParameterParser.h
	inc/MantidAPI/ImplicitFunctionParameterParserFactory.h
	inc/MantidAPI/ImplicitFunctionParser.h
	inc/MantidAPI/ImplicitFunctionParserFactory.h
	inc/MantidAPI/InstrumentDataService.h
	inc/MantidAPI/Jacobian.h
	inc/MantidAPI/JointDomain.h
	inc/MantidAPI/LinearScale.h
	inc/MantidAPI/LiveListenerFactory.h
	inc/MantidAPI/LogManager.h
	inc/MantidAPI/LogarithmScale.h
	inc/MantidAPI/MDGeometry.h
	inc/MantidAPI/MatrixWSIndexCalculator.h
	inc/MantidAPI/MatrixWorkspace.h
	inc/MantidAPI/MatrixWorkspaceMDIterator.h
	inc/MantidAPI/MemoryManager.h
	inc/MantidAPI/ModeratorModel.h
	inc/MantidAPI/MultiDomainFunction.h
	inc/MantidAPI/MultiPeriodGroupAlgorithm.h
	inc/MantidAPI/MultipleExperimentInfos.h
	inc/MantidAPI/MultipleFileProperty.h
	inc/MantidAPI/NullCoordTransform.h
	inc/MantidAPI/NumericAxis.h
	inc/MantidAPI/ParamFunction.h
	inc/MantidAPI/ParameterReference.h
	inc/MantidAPI/ParameterTie.h
	inc/MantidAPI/Progress.h
	inc/MantidAPI/PropertyManagerDataService.h
	inc/MantidAPI/PropertyNexus.h
	inc/MantidAPI/RefAxis.h
	inc/MantidAPI/Run.h
	inc/MantidAPI/Sample.h
	inc/MantidAPI/SampleEnvironment.h
	inc/MantidAPI/ScopedWorkspace.h
	inc/MantidAPI/ScriptRepository.h
	inc/MantidAPI/ScriptRepositoryFactory.h
	inc/MantidAPI/SingleValueParameter.h
	inc/MantidAPI/SingleValueParameterParser.h
	inc/MantidAPI/SpecialCoordinateSystem.h
	inc/MantidAPI/SpectraAxis.h
	inc/MantidAPI/SpectrumDetectorMapping.h
	inc/MantidAPI/TableRow.h
	inc/MantidAPI/TextAxis.h
	inc/MantidAPI/TransformScaleFactory.h
	inc/MantidAPI/VectorParameter.h
	inc/MantidAPI/VectorParameterParser.h
	inc/MantidAPI/Workspace.h
	inc/MantidAPI/WorkspaceFactory.h
	inc/MantidAPI/WorkspaceGroup.h
	inc/MantidAPI/WorkspaceHistory.h
	inc/MantidAPI/WorkspaceOpOverloads.h
	inc/MantidAPI/WorkspaceProperty.h
	inc/MantidAPI/WorkspaceValidators.h
)

set ( TEST_FILES
	#	IkedaCarpenterModeratorTest.h
	AlgorithmFactoryTest.h
	AlgorithmHasPropertyTest.h
	AlgorithmHistoryTest.h
	AlgorithmManagerTest.h
	AlgorithmPropertyTest.h
	AlgorithmProxyTest.h
	AlgorithmTest.h
	AnalysisDataServiceTest.h
	AsynchronousTest.h
	BoxControllerTest.h
	CompositeFunctionTest.h
	CoordTransformTest.h
	CostFunctionFactoryTest.h
	DataProcessorAlgorithmTest.h
	EnabledWhenWorkspaceIsTypeTest.h
	ExperimentInfoTest.h
	ExpressionTest.h
	FermiChopperModelTest.h
	FileFinderTest.h
	FilePropertyTest.h
	FrameworkManagerTest.h
	FuncMinimizerFactoryTest.h
	FunctionAttributeTest.h
	FunctionDomainTest.h
	FunctionFactoryTest.h
	FunctionPropertyTest.h
	FunctionTest.h
	FunctionValuesTest.h
	IEventListTest.h
	IFunction1DTest.h
	IFunctionMDTest.h
	ILiveListenerTest.h
	IMDWorkspaceTest.h
	ISpectrumTest.h
	IkedaCarpenterModeratorTest.h
	ImmutableCompositeFunctionTest.h
	ImplicitFunctionParserFactoryTest.h
	IncreasingAxisValidatorTest.h
	InstrumentDataServiceTest.h
	LiveListenerFactoryTest.h
	LogManagerTest.h
	MDGeometryTest.h
	MatrixWorkspaceMDIteratorTest.h
	MemoryManagerTest.h
	ModeratorModelTest.h
	MultiDomainFunctionTest.h
	MultiPeriodGroupAlgorithmTest.h
	MultipleExperimentInfosTest.h
	MultipleFilePropertyTest.h
	NumericAxisTest.h
	ParamFunctionAttributeHolderTest.h
	ParameterReferenceTest.h
	ParameterTieTest.h
	PropertyManagerDataServiceTest.h
	PropertyNexusTest.h
	RunTest.h
	SampleEnvironmentTest.h
	SampleTest.h
	ScopedWorkspaceTest.h
	SpectraAxisTest.h
	SpectrumDetectorMappingTest.h
	TextAxisTest.h
	VectorParameterParserTest.h
	VectorParameterTest.h
	WorkspaceFactoryTest.h
	WorkspaceGroupTest.h
	WorkspaceHistoryTest.h
	WorkspaceOpOverloadsTest.h
	WorkspacePropertyTest.h
)

set ( GMOCK_TEST_FILES
        ImplicitFunctionFactoryTest.h
        ImplicitFunctionParameterParserFactoryTest.h
	MatrixWorkspaceTest.h
)


if(UNITY_BUILD)
  include(UnityBuild)
  enable_unity_build(API SRC_FILES SRC_UNITY_IGNORE_FILES 10)
endif(UNITY_BUILD)

# Have to link to winsock library on Windows
if ( WIN32 )
  set ( WINSOCK ws2_32 )
endif ()

# Add a precompiled header where they are supported
enable_precompiled_headers( inc/MantidAPI/PrecompiledHeader.h SRC_FILES )
# Add the target for this directory
add_library ( API ${SRC_FILES} ${INC_FILES})
# Set the name of the generated library
set_target_properties ( API PROPERTIES OUTPUT_NAME MantidAPI 
                                       COMPILE_DEFINITIONS IN_MANTID_API )
# Add to the 'Framework' group in VS
set_property ( TARGET API PROPERTY FOLDER "MantidFramework" )

target_link_libraries ( API ${MANTIDLIBS} ${WINSOCK} ${NEXUS_LIBRARIES} )

# Add the unit tests directory
add_subdirectory ( test )

###########################################################################
# Installation settings
###########################################################################

install ( TARGETS API ${SYSTEM_PACKAGE_TARGET} DESTINATION ${LIB_DIR} )
