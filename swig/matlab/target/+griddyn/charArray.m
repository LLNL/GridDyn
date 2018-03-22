classdef charArray < SwigRef
  methods
    function this = swig_this(self)
      this = griddynMEX(3, self);
    end
    function self = charArray(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = griddynMEX(21, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.swigPtr = [];
      end
    end
    function delete(self)
      if self.swigPtr
        griddynMEX(22, self);
        self.swigPtr=[];
      end
    end
    function varargout = paren(self,varargin)
      [varargout{1:nargout}] = griddynMEX(23, self, varargin{:});
    end
    function varargout = paren_asgn(self,varargin)
      [varargout{1:nargout}] = griddynMEX(24, self, varargin{:});
    end
    function varargout = cast(self,varargin)
      [varargout{1:nargout}] = griddynMEX(25, self, varargin{:});
    end
  end
  methods(Static)
    function varargout = frompointer(varargin)
     [varargout{1:nargout}] = griddynMEX(26, varargin{:});
    end
  end
end
