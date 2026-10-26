;;; Directory Local Variables            -*- no-byte-compile: t -*-
;;; For more information see (info "(emacs) Directory Variables")
;; The code is C89-style, two-space indents, no tabs (see AGENTS.md).
((nil . ((compile-command . "make cli")
         (fill-column . 80)
         ;; Attach dape to the STM32 through OpenOCD's gdb server. Start
         ;; OpenOCD first, then M-x dape and pick a config. The ELF must match
         ;; what is flashed: `make flash' pairs with stm32-attach, and
         ;; `make debug-flash' pairs with stm32-attach-debug (locals visible,
         ;; but the -O0 ISR is too slow for clean audio).
         ;; Keys without a colon are dape's own; :keys go to lldb-dap, which
         ;; wants gdb-remote-port for this kind of attach.
         (dape-configs
          . ((stm32-attach
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
