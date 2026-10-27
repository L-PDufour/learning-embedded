;;; Directory Local Variables            -*- no-byte-compile: t -*-
;;; For more information see (info "(emacs) Directory Variables")
;; The code is C89-style, two-space indents, no tabs (see AGENTS.md).
((nil . ((compile-command . "make cli")
         (fill-column . 80)
         ;; dape configs, host first. Keys without a colon are dape's own;
         ;; :keys go straight into the DAP request.
         ;;
         ;; host-debug - the fast loop for engine and filter work: same
         ;; engine/*.c as the firmware, -O0, no ISR, no board.
         ;;   * :program is ./synth, the binary `make cli' builds. Line
         ;;     breakpoints only bind if it has -g, and the Makefile is not a
         ;;     prerequisite of the synth target, so after any CFLAGS change
         ;;     run `make -B cli'. Otherwise `br s -f filter.c' stays pending
         ;;     and you silently debug an image with no symbols.
         ;;   * :stdio is accepted by lldb-dap but IGNORED (verified with
         ;;     lldb-dap 21.1.8): the inferior always gets a pty, and LLDB 21
         ;;     dropped target.process.output-path, so platform_cli's audio
         ;;     arrives as DAP "output" events. The CLI has no clock of its
         ;;     own (aplay's pipe is what throttles it), so with no reader it
         ;;     spins out ~130 MB/s: never free-run it. Stop on entry, set a
         ;;     breakpoint, then exactly one `continue'.
         ;;   * The filter is bypassed until something sets the type:
         ;;     filter_init() hardcodes FILTER_NONE and there is no setter.
         ;;     At the first stop, in the dape REPL (~\*dape-repl\*), type:
         ;;       eval e.filter.type = (FilterType)2
         ;;     0 NONE, 1 BAND_PASS, 2 LOW_PASS, 3 HIGH_PASS.
         ;;   * Prefer `eval e.filter' to frame locals: at -O0 clang ends a
         ;;     local's DWARF range after its last use, so `eval q' inside
         ;;     filter_process often reports "no location".
         ;;
         ;; stm32-attach / stm32-attach-debug - the board. Start OpenOCD
         ;; first, then M-x dape. The ELF must match what is flashed:
         ;; `make flash' pairs with stm32-attach, and `make debug-flash'
         ;; pairs with stm32-attach-debug (locals visible, but the -O0 ISR is
         ;; too slow for clean audio). lldb-dap wants gdb-remote-port for
         ;; this kind of attach.
         (dape-configs
          . ((host-debug
              modes (c-mode c-ts-mode)
              command "lldb-dap"
              :type "lldb-dap"
              :request "launch"
              :program "/home/desktop/Documents/synth/synth"
              :cwd "/home/desktop/Documents/synth"
              :stopOnEntry t)
             (stm32-attach
              modes (c-mode c-ts-mode)
              command "lldb-dap"
              :type "lldb-dap"
              :request "attach"
              :program "/home/desktop/Documents/synth/synth.elf"
              :gdb-remote-hostname "localhost"
              :gdb-remote-port 3333)
             (stm32-attach-debug
              modes (c-mode c-ts-mode)
              command "lldb-dap"
              :type "lldb-dap"
              :request "attach"
              :program "/home/desktop/Documents/synth/synth-debug.elf"
              :gdb-remote-hostname "localhost"
              :gdb-remote-port 3333)))))
 (c-mode . ((c-basic-offset . 2)
            (indent-tabs-mode . nil)
            (tab-width . 8)))
 (c-ts-mode . ((c-ts-mode-indent-offset . 2)
               (indent-tabs-mode . nil)
               (tab-width . 8))))
