// Make headings collapsible
document.addEventListener('DOMContentLoaded', function() {
  const isFilePreview = window.location.protocol === 'file:';
  if (isFilePreview) {
    document.documentElement.classList.add('file-preview-no-search');
  }

  const searchToggle = document.getElementById('__search');
  if (searchToggle && !isFilePreview) {
    searchToggle.checked = true;
  }

  const headings = document.querySelectorAll('.md-typeset h2, .md-typeset h3');
  
  headings.forEach(heading => {
    heading.addEventListener('click', function(e) {
      // Don't trigger on anchor link clicks
      if (e.target.tagName === 'A') return;
      
      this.classList.toggle('collapsed');
      
      // Collapse/expand all siblings until the next heading of same or higher level
      let sibling = this.nextElementSibling;
      const currentLevel = parseInt(this.tagName[1]);
      
      while (sibling) {
        // Stop at headings of same or higher level
        if (sibling.tagName && /^H[1-3]$/.test(sibling.tagName)) {
          const siblingLevel = parseInt(sibling.tagName[1]);
          if (siblingLevel <= currentLevel) break;
        }
        
        if (this.classList.contains('collapsed')) {
          sibling.style.display = 'none';
        } else {
          sibling.style.display = '';
        }
        
        sibling = sibling.nextElementSibling;
      }
    });
  });
});
