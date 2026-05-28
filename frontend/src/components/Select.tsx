import { useState, useRef, useEffect } from "react";
import type { KeyboardEvent } from "react";

interface SelectOption {
  value: string;
  label: string;
}

interface SelectProps {
  value: string;
  onChange: (value: string) => void;
  options: SelectOption[];
  placeholder?: string;
  className?: string;
  style?: React.CSSProperties;
}

export default function Select({
  value,
  onChange,
  options,
  placeholder,
  className = "",
  style,
}: SelectProps) {
  const [open, setOpen] = useState(false);
  const ref = useRef<HTMLDivElement>(null);
  const selected = options.find((o) => o.value === value);

  useEffect(() => {
    const handler = (e: MouseEvent) => {
      if (ref.current && !ref.current.contains(e.target as Node)) {
        setOpen(false);
      }
    };
    document.addEventListener("mousedown", handler);
    return () => document.removeEventListener("mousedown", handler);
  }, []);

  const handleKeyDown = (e: KeyboardEvent) => {
    if (e.key === "Enter" || e.key === " ") {
      setOpen(!open);
    } else if (e.key === "Escape") {
      setOpen(false);
    } else if (e.key === "ArrowDown" || e.key === "ArrowUp") {
      e.preventDefault();
      const idx = options.findIndex((o) => o.value === value);
      const next =
        e.key === "ArrowDown"
          ? Math.min(idx + 1, options.length - 1)
          : Math.max(idx - 1, 0);
      onChange(options[Math.max(0, next)].value);
    }
  };

  return (
    <div
      ref={ref}
      className={`custom-select ${className}`}
      style={{ position: "relative", ...style }}
      tabIndex={0}
      role="listbox"
      aria-expanded={open}
      onKeyDown={handleKeyDown}
    >
      <div
        className="custom-select-trigger"
        onClick={() => setOpen(!open)}
        style={{
          background: "var(--bg-input)",
          border: "1px solid var(--border)",
          color: "var(--text-primary)",
          padding: "6px 12px",
          borderRadius: "4px",
          fontSize: "14px",
          cursor: "pointer",
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
          userSelect: "none",
        }}
      >
        <span>{selected ? selected.label : placeholder || "Select..."}</span>
        <span style={{ marginLeft: "8px", opacity: 0.5, fontSize: "11px" }}>&#9662;</span>
      </div>
      {open && (
        <div
          className="custom-select-dropdown"
          style={{
            position: "absolute",
            top: "100%",
            left: 0,
            right: 0,
            zIndex: 1000,
            background: "var(--bg-surface)",
            border: "1px solid var(--border-light)",
            borderRadius: "4px",
            marginTop: "2px",
            maxHeight: "200px",
            overflowY: "auto",
          }}
        >
          {options.map((opt) => (
            <div
              key={opt.value}
              role="option"
              aria-selected={opt.value === value}
              onClick={() => {
                onChange(opt.value);
                setOpen(false);
              }}
              style={{
                padding: "8px 12px",
                cursor: "pointer",
                color: "var(--text-primary)",
                background:
                  opt.value === value ? "var(--bg-hover)" : "transparent",
                fontSize: "14px",
              }}
              onMouseEnter={(e) => {
                (e.target as HTMLElement).style.background = "var(--bg-hover)";
              }}
              onMouseLeave={(e) => {
                (e.target as HTMLElement).style.background =
                  opt.value === value ? "var(--bg-hover)" : "transparent";
              }}
            >
              {opt.label}
            </div>
          ))}
        </div>
      )}
    </div>
  );
}
