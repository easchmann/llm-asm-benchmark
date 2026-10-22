def heading(text):
    print(f"## {text}\n")


def table(header, rows):
    print("| " + " | ".join(str(h) for h in header) + " |")
    print("|" + "---|" * len(header))
    for row in rows:
        print("| " + " | ".join(str(c) for c in row) + " |")
    print()
