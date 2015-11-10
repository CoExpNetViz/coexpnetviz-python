def is_ancestor(path, sub_path):
    '''Get whether first plumbum path is ancestor of the second'''
    relative = sub_path.relative_to(path)
    return len(relative) > 0 and relative[0] != '..'

def is_ancestor_or_equal(path, sub_path):
    '''Get whether first plumbum path is ancestor or equal to the second'''
    return path == sub_path or is_ancestor(path, sub_path)
