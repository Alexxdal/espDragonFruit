import * as React from 'react';
import { XIcon } from 'lucide-react';
import { cn } from '@/lib/utils';

interface SheetProps {
  open: boolean;
  onOpenChange: (open: boolean) => void;
  children: React.ReactNode;
  side?: 'top' | 'right' | 'bottom' | 'left';
}

function Sheet({ open, children }: SheetProps) {
  return <>{open && children}</>;
}

function SheetTrigger({
  onClick,
  children,
  ...props
}: React.ButtonHTMLAttributes<HTMLButtonElement>) {
  return (
    <button
      {...props}
      onClick={(e) => {
        onClick?.(e);
      }}
      data-slot='sheet-trigger'
    >
      {children}
    </button>
  );
}

function SheetClose({
  onClick,
  ...props
}: React.ButtonHTMLAttributes<HTMLButtonElement> & { onClose?: () => void }) {
  return (
    <button
      {...props}
      onClick={(e) => {
        onClick?.(e);
        props.onClose?.();
      }}
      data-slot='sheet-close'
    >
      Close
    </button>
  );
}

function SheetOverlay({ onClick }: { onClick?: () => void }) {
  return (
    <div
      data-slot='sheet-overlay'
      className='fixed inset-0 z-40 bg-black/50'
      onClick={onClick}
    />
  );
}

function SheetContent({
  open,
  onOpenChange,
  children,
  side = 'right'
}: {
  open: boolean;
  onOpenChange: (open: boolean) => void;
  children: React.ReactNode;
  side?: 'top' | 'right' | 'bottom' | 'left';
}) {
  if (!open) return null;

  let positionClasses = '';
  switch (side) {
    case 'right':
      positionClasses = 'inset-y-0 right-0 h-full w-3/4 sm:max-w-sm';
      break;
    case 'left':
      positionClasses = 'inset-y-0 left-0 h-full w-3/4 sm:max-w-sm';
      break;
    case 'top':
      positionClasses = 'inset-x-0 top-0 h-auto w-full';
      break;
    case 'bottom':
      positionClasses = 'inset-x-0 bottom-0 h-auto w-full';
      break;
  }

  return (
    <>
      <SheetOverlay onClick={() => onOpenChange(false)} />
      <div
        data-slot='sheet-content'
        className={cn(
          'fixed z-50 flex flex-col gap-4 bg-background shadow-lg transition-transform',
          positionClasses
        )}
      >
        {children}
        <button
          className='absolute top-4 right-4 rounded opacity-70 hover:opacity-100'
          onClick={() => onOpenChange(false)}
        >
          <XIcon className='size-4' />
          <span className='sr-only'>Close</span>
        </button>
      </div>
    </>
  );
}

function SheetHeader({ className, ...props }: React.ComponentProps<'div'>) {
  return (
    <div
      data-slot='sheet-header'
      className={cn('flex flex-col gap-1.5 p-4', className)}
      {...props}
    />
  );
}

function SheetFooter({ className, ...props }: React.ComponentProps<'div'>) {
  return (
    <div
      data-slot='sheet-footer'
      className={cn('mt-auto flex flex-col gap-2 p-4', className)}
      {...props}
    />
  );
}

function SheetTitle({
  className,
  children,
  ...props
}: {
  className?: string;
  children: React.ReactNode;
}) {
  return (
    <h2
      data-slot='sheet-title'
      className={cn('text-foreground font-semibold', className)}
      {...props}
    >
      {children}
    </h2>
  );
}

function SheetDescription({
  className,
  children,
  ...props
}: {
  className?: string;
  children: React.ReactNode;
}) {
  return (
    <p
      data-slot='sheet-description'
      className={cn('text-muted-foreground text-sm', className)}
      {...props}
    >
      {children}
    </p>
  );
}

export {
  Sheet,
  SheetTrigger,
  SheetClose,
  SheetContent,
  SheetHeader,
  SheetFooter,
  SheetTitle,
  SheetDescription
};
