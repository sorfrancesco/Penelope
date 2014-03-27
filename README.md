How to run Penelope.
========

Prediction Testing Tool

    Monitor
    Use clean class files (not already instrumented)
    The tool was tuned on Java 1.6 (for Java 1.7 please use the option "-XX:-UseSplitVerifier")
    run "java Monitor -U mainclass.class File1.class File2.class"
    run "java program"
    

    The output of this phase is program.monitored and will contain the observed run.
        Example of observed run (VT3)
        Details about the observed run (README1.rtf)


    Predict (Preprocessing/Prefix/File Generation for the SMT)
    run "./prediction VT3"
    After this process different files are created:
        VT3_mod.nrun that is the observed run after the hashing of the methods and the variables, enriched with localev number and global event number.
        VT3_mod.pattern that contains the patterns found in the prediction in the format T1 T2 e1 e e2 f
        VT3_hvar that contains the hashing used for the variables
        nest_conf.h that contains the number of vars, locks and threads involved in the observed run
        VT3_mod.patternSMT that contains the patterns found in the prediction for the SMT solver (without region A)
        a bunch of files Program_mod.nrun_smt_# that are the set of runs passed to the SMT solver
        a bunch of files Program_mod.nrun_val_# that contain initial values of the variables associated with the files VT3_mod.nrun_smt_#


    Predict (SMT)
    ... 


    Rescheduling
    use clean class files (not already instrumented)
    run "java Scheduler -U #threads_involved mainclass.class File1.class File2.class"
    ... 
