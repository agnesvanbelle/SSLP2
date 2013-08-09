SSLP2
=====

Source-side reordering using a Probalistic Inverse Transduction Grammar (PITG).

Source-side reordering is a Statistical Machine Translation (SMT) approach to account for differences 
in source and target language word order by imposing a reordering model on the source data as a pre-translation step.

To construct a reordering model various grammars can be used, in this paper we focus on the use of an Inversion
Transduction Grammar (ITG). To constrain the model in a linguistically motivated we we impose source-side syntax
constraints on the ITG rules that can be derived.

The results of our model given a reordered gold standard on two well-known sentence distance metrics 
can be examined and analyzed using BitPar.
