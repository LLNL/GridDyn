# SyntheticUSA PSS/E compatibility assessment

## Scope and source files

This assessment covers the static SyntheticUSA package supplied at
`C:\Users\phlpt\Documents\test_cases\SyntheticUSA`. It contains the PSS/E v33
`SyntheticUSA.RAW`, a PowerWorld `SyntheticUSA.EPC` reference, and matching
`.pwb`/`.pwd` files. `case_SyntheticUSA.m` is the available MATPOWER reference.
There is no DYR file or other dynamic-model data, so this case is a
power-flow/topology compatibility target only; it must not be represented as a
dynamic simulation reference.

GridDyn's import/validation path is `SyntheticUSA.RAW`. The EPC, PWB, PWD, and
MATPOWER files are independent reference representations for comparing the
imported topology and solved state.

## Static inventory

| Item                        | Source count | GridDyn requirement                                                                                     |
| --------------------------- | -----------: | ------------------------------------------------------------------------------------------------------- |
| Terminal buses              |       79,600 | Standard PSS/E v33 bus import.                                                                          |
| Loads                       |       44,430 | Standard ZIP/load import and bus attachment.                                                            |
| Fixed shunts                |          916 | Fixed-shunt import.                                                                                     |
| Generators                  |       13,419 | Standard generator, reactive-limit, and voltage-control import.                                         |
| AC branches                 |       83,425 | Standard branch import.                                                                                 |
| Two-winding transformers    |       13,496 | Transformer impedance, rating, tap, phase shift, and control import.                                    |
| Three-winding transformers  |        2,400 | Delta-to-star expansion into three transformer legs.                                                    |
| Areas / zones / owners      |  76 / 28 / 1 | Metadata or transfer-control preservation where applicable; not new AC elements.                        |
| Two-terminal DC links       |            9 | Existing `RawDcLine` compatibility import; validate scheduled transfer and terminal reactive treatment. |
| Impedance-correction tables |            4 | Existing interpolation support, including all referenced transformer table IDs.                         |
| Switched shunts             |        4,065 | Existing v33 switched-shunt parsing, block expansion, control mode, and initial `BINIT` treatment.      |

The RAW therefore contains 79,600 terminal buses and 83,425 ordinary AC
branches. Adding 13,496 two-winding transformer links and three legs for each
of the 2,400 three-winding units gives the 82,000-bus, 104,121-branch topology
in `case_SyntheticUSA.m`.

The VSC DC, multi-terminal DC, multi-section-line, FACTS, and GNE sections are
empty. No unsupported nonempty RAW section was found. The nonempty
two-terminal-DC section spans 27 physical lines because PSS/E encodes each of
the nine links as one DC-line record plus two converter records.

## Power-flow run status (2026-10-19)

The MATPOWER `.m` case solves. The RAW and EPC files import substantial parts
of the same network but both fail in KINSOL during the initial power-flow
solve. No dynamic-model data is present, so this package remains a static
power-flow and topology target.

| Input                 | Result                                                                        | Imported network                                                          | Finding                                                                                                                                                                                                                                                                                                                                  |
| --------------------- | ----------------------------------------------------------------------------- | ------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `case_SyntheticUSA.m` | Solves, including with `no_powerflow_adjustments,no_powerflow_error_recovery` | 82,000 buses, 104,121 links, 13,419 generators, 39,993 loads              | Exported solution residuals at CSV precision: max P 0 MW, max Q 0.0002 MVAr; voltage range 0.937407–1.0900 pu. This is the working static baseline.                                                                                                                                                                                      |
| `SyntheticUSA.RAW`    | Fails with KINSOL `-11` (linear solver setup failed unrecoverably)            | 82,000 buses, 104,130 links, 13,419 generators, 49,411 load/shunt objects | Strict run also fails. Removing the nine two-terminal DC records did not resolve it. Setting all generator `IREG` fields to zero, including 2,286 online remote targets, did not resolve it either. The specific singularity or Jacobian defect remains unidentified.                                                                    |
| `SyntheticUSA.EPC`    | Fails with KINSOL `-11`                                                       | 82,018 buses, 104,130 links, 13,419 generators, 49,411 load/shunt objects | The reader reports an unsupported nonempty DC converter section and disconnects 18 buses across nine networks without a slack or PV bus. The 18 extra buses come from the EPC `dc bus data [18]` section and have no solved voltages. Fixing EPC DC interpretation is required before the solver failure can be diagnosed independently. |

The `.m` and RAW imports each report 82,000 buses, but their bus-ID sets are
not identical: 300 generated RAW star buses (81702–82001) have no MATPOWER ID,
and 300 IDs appear only in MATPOWER (including IDs such as 2010068). Resolve
this representation difference before treating the files as a direct
bus-by-bus parity pair. The RAW/EPC object count of 49,411 includes 44,430
loads, 916 fixed shunts, and 4,065 switched shunts; MATPOWER reports 39,993
bus-level loads. These counts reflect different load/shunt representations
and are not a direct load-count equivalence check.

Neither tested RAW simplification—removing the nine DC lines or clearing all
`IREG` fields—changes the KINSOL `-11` outcome. The run artifacts and
pre-solve exports are in the repository-local `tmp/` directory
(`SyntheticUSA_matpower_solution.csv`, `SyntheticUSA_raw_pre.csv`, and
`SyntheticUSA_epc_pre.csv`, with corresponding console logs).

### Follow-up work

1. Fix EPC DC converter and DC-bus import so the nine DC networks do not
   create 18 orphan AC buses; rerun EPC power flow afterward.
2. Diagnose the RAW KINSOL linear setup failure on the fully imported network.
   Start by isolating connectivity/Jacobian singularity sources around the
   expanded three-winding network and remote voltage controls; the two tested
   simplifications above do not explain the failure.
3. Align RAW, EPC, and MATPOWER bus identities and compare the solved MATPOWER
   state against successful RAW/EPC runs, including all three-winding
   terminals, the nine DC transfers, impedance-correction tables, switched
   shunts, P/Q injections, and branch/transformer flows.
4. Add regression coverage only after the RAW/EPC solver paths converge and
   the cross-format comparison uses an explicitly mapped set of buses.

The `.pwb` and `.pwd` files are present as references, but GridDyn does not
currently dispatch those extensions as input formats. Current static-run
follow-ups are also tracked in
[Power-flow validation follow-ups](powerflow-validation-followups.md).
