#!/usr/bin/env python3
# encoding=utf-8

"""
This module provides a command line tool for processing icon font metadata.
"""

from inspect import cleandoc
from pathlib import Path
from typing  import List, Tuple

import json
import re
import sys


IconList = List[Tuple[str, str|None, str|None]]


def make_enumkey(name: str) -> str:
    """
    Converts `name` into a camelcase enum key.
    The result gets prefixed with "_" if `name` starts with a number.
    """

    sections =  [
        s[0].upper() + s[1:].lower()
        for s in re.split(r'[ \t_-]', name)
    ]

    name = ''.join(sections)

    if name[0].isdigit():
        name = f'_{name}'

    return name


def print_icon_definitions(iconlist: IconList) -> None:
    """
    Prints the icon definitions in `iconlist`.
    The tuples of this list are the name, value and comment for the icon.
    Value and comment are optional, and therefore can be `None`.
    """

    nw = max(
        len(name) if name else 0
        for name, _, _ in iconlist
    )

    vw = max(
        len(value) if value else 0
        for _, value, _ in iconlist
    ) + 1

    for name, value, comment in iconlist:
        if value is not None:
            if comment:
                print(f'    {name:<{nw}} = {f"{value},":<{vw}} // {comment}')

            else:
                print(f'    {name:<{nw}} = {value},')

        else:
            if comment:
                print(f' // {name:<{nw}} - {"<unsupported>,":{vw + 3}} {comment}')

            else:
                print(f' // {name:<{nw}} - <unsupported>')


def deduplicate_icons(icons: IconList) -> IconList:
    """
    Renames duplicate icon names to avoid name conflicts.
    This is needed with fonts like Google's "Material Icons Regular".
    """

    # first track at which index icon names occur

    iconname_indexes: dict[str, list[int]] = {}

    for i, info in enumerate(icons):
        name, _, _ = info

        indexes = iconname_indexes.get(name, [])
        iconname_indexes[name] = indexes + [i]

    # duplicates have more than one occurance

    duplicates = {
        name: indexes for name, indexes
        in iconname_indexes.items() if len(indexes) > 1
    }

    # produce unique names for duplicates

    def try_name(name: str) -> str | None:
        if not iconname_indexes.get(name):
            return name

        return None

    for name, indexes in duplicates.items():
        for index in indexes[1:]:
            new_name = try_name(f'{name}Alt')
            counter  = 1

            # create unique name by counting upwards

            while new_name is None:
                new_name = try_name(f'{name}Alt{counter}')
                counter += 1

            # store the new name

            iconname_indexes[new_name] = [index]
            icons[index] = (new_name, *icons[index][1:])

    # report result

    return icons


def convert_fontawesome_metadata(filepath: Path, license_name: str, style_name: str) -> IconList:
    """
    Transforms the Font-Awesome-style icon metadata in `filepath` into an `IconList`,
    where `license_name` and `style` are used to filter the list.
    """

    icons: IconList = []

    with open(filepath, mode='rt', encoding='utf-8') as file:
        for name, info in json.load(file).items():
            codepoint = info['unicode']

            supported_styles_key = 'free' if license_name == 'free' else 'styles'
            supported_styles = info[supported_styles_key]

            name = make_enumkey(name)
            comment = f'supported styles: {", ".join(supported_styles)}'

            if style_name not in supported_styles:
                icons.append((name, None, comment))
                continue

            icons.append((name, f'0x{codepoint}', comment))

            for alias in info.get('aliases', {}).get('names', []):
                icons.append((make_enumkey(alias), name, comment))

    return icons


def convert_icomoon_selection(filepath: Path) -> IconList:
    """
    Transforms the IcoMoon selection in `filepath` into an `IconList`.
    """

    icons: IconList = []

    with open(filepath, mode='rt', encoding='utf-8') as file:
        selection = json.load(file)

        if selection.get('IcoMoonType') != 'selection':
            raise ValueError('Unsupported file type')

        for icon in selection['icons']:
            name = make_enumkey(icon['properties']['name'])
            codepoint = int(icon['properties']['code'])
            comments: list[str] = []

            if tags := ', '.join(icon['icon'].get('tags', [])):
                comments.append(f'tags: {tags}')

            icons.append((name, f'0x{codepoint:x}', ', '.join(comments)))

    return icons


def convert_materialsymbols_codepoints(filepath: Path) -> IconList:
    """
    Transforms the Material-Symbols-style icon metadata in `filepath` into an `IconList`.
    """

    icons: IconList = []

    with open(filepath, mode='rt', encoding='utf-8') as file:
        for line in file:
            name, codepoint = line.split()
            name = make_enumkey(name)

            icons.append((name, f'0x{codepoint}', None))

    return icons


def print_usage(error_message: str) -> None:
    """
    Prints usage information for the tool.
    """

    print(cleandoc(f'''
        Usage error: {error_message}

        Usage:
            {sys.argv[0]} metadata.json license style
            {sys.argv[0]} metadata.codepoints
    '''), file=sys.stderr)

    sys.exit(2)


def main(args: list[str]) -> None:
    """
    Main routine of the tool, where `args` is a list of command line options.
    """

    if len(args) < 2:
        print_usage('The filename is missing')

    filepath = Path(args[1])

    if filepath.name == 'selection.json':
        icons = convert_icomoon_selection(filepath)
        icons = deduplicate_icons(icons)
        print_icon_definitions(icons)

    elif filepath.name.endswith('.json'):
        if len(args) < 4:
            print_usage('The required arguments are missing')

        icons = convert_fontawesome_metadata(filepath, *args[2:4])
        icons = deduplicate_icons(icons)
        print_icon_definitions(icons)

    elif filepath.name.endswith('.codepoints'):
        icons = convert_materialsymbols_codepoints(filepath)
        icons = deduplicate_icons(icons)
        print_icon_definitions(icons)

    else:
        print_usage('Unsupported filename')

if __name__ == '__main__':
    main(sys.argv)
