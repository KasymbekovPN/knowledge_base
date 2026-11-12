;;; config.el -*- lexical-binding: t; -*-
;;
;; Личные настройки. Изменения применяются после перезапуска Emacs
;; (или M-x doom/reload). doom sync нужен только после правки init.el / packages.el.

(setq user-full-name "Pablo")

;;; ─── Внешний вид ──────────────────────────────────────────────────────────
(setq doom-font (font-spec :family "JetBrains Mono" :size 15)
      doom-theme 'doom-one
      display-line-numbers-type 'relative)

;;; ─── Проекты ──────────────────────────────────────────────────────────────
;; Код держи в файловой системе Linux (~/projects), а не в /mnt/c — в разы быстрее.
(setq projectile-project-search-path '(("~/projects" . 2)))

;;; ─── Производительность LSP ───────────────────────────────────────────────
(setq read-process-output-max (* 4 1024 1024))

(after! eglot
  (setq eglot-events-buffer-config '(:size 0 :format full) ; не логировать JSON-RPC
        eglot-autoshutdown t                               ; гасить сервер, когда закрыт последний буфер
        eglot-extend-to-xref t)                            ; оставаться в LSP при переходе в заголовки

  ;; clangd под большой монолит. -j подстрой под число ядер, выделенных WSL.
  (add-to-list 'eglot-server-programs
               '((c-mode c-ts-mode c++-mode c++-ts-mode)
                 . ("clangd"
                    "-j=8"
                    "--background-index"
                    "--clang-tidy"
                    "--completion-style=detailed"
                    "--header-insertion=never"
                    "--pch-storage=memory")))

  ;; rust-analyzer: проверка через clippy, все features
  (setq-default eglot-workspace-configuration
                '(:rust-analyzer (:check (:command "clippy")
                                  :cargo (:features "all")))))

;;; ─── Сборка ───────────────────────────────────────────────────────────────
;; SPC c c — compile, затем ]e / [e или M-g n / M-g p по ошибкам
(setq compilation-scroll-output 'first-error)

;;; ─── Отладка: dape (DAP) ──────────────────────────────────────────────────
;; gdb 14+ умеет DAP нативно (в Ubuntu 24.04 gdb 15) — отдельный адаптер не нужен.
(use-package! dape
  :defer t
  :config
  (setq dape-buffer-window-arrangement 'right))

(map! :leader
      (:prefix ("d" . "debug")
       :desc "Start / attach"     "d" #'dape
       :desc "Toggle breakpoint"  "b" #'dape-breakpoint-toggle
       :desc "Continue"           "c" #'dape-continue
       :desc "Next"               "n" #'dape-next
       :desc "Step in"            "s" #'dape-step-in
       :desc "Step out"           "o" #'dape-step-out
       :desc "Restart"            "r" #'dape-restart
       :desc "Quit"               "q" #'dape-quit))

;;; ─── Magit на большом репозитории ─────────────────────────────────────────
(after! magit
  (setq magit-diff-refine-hunk t)
  ;; Если status открывается медленно — эти секции самые дорогие:
  (remove-hook 'magit-status-sections-hook #'magit-insert-tags-header))
